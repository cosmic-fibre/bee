/*
 * lyaml.c, LibYAML binding for Lua
 *
 * Copyright (c) 2009, Andrew Danforth <acd@weirdness.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Portions of this software were inspired by Perl's YAML::LibYAML module by
 * Ingy döt Net <ingy@cpan.org>
 *
 */

#include "lyaml.h"

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <lj_obj.h>
#include <lj_ctype.h>
#include <lj_cdata.h>
#include <lj_cconv.h>
#include <lj_state.h>

#include "yaml.h"
#include "b64.h"
} /* extern "C" */
#include "lua/utils.h"

#define LUAYAML_TAG_PREFIX "tag:yaml.org,2002:"

#define RETURN_ERRMSG(s, msg) do { \
      lua_pushstring(s->L, msg); \
      s->error = 1; \
      return; \
   } while(0)

struct lua_yaml_loader {
   lua_State *L;
   struct luaL_serializer *cfg;
   int anchortable_index;
   int document_count;
   yaml_parser_t parser;
   yaml_event_t event;
   char validevent;
   char error;
};

struct lua_yaml_dumper {
   lua_State *L;
   struct luaL_serializer *cfg;
   int anchortable_index;
   unsigned int anchor_number;
   yaml_emitter_t emitter;
   char error;

   lua_State *outputL;
   luaL_Buffer yamlbuf;
};

static void generate_error_message(struct lua_yaml_loader *loader) {
   char buf[256];
   luaL_Buffer b;

   luaL_buffinit(loader->L, &b);

   luaL_addstring(&b, loader->parser.problem ? loader->parser.problem : "A problem");
   snprintf(buf, sizeof(buf), " at document: %d", loader->document_count);
   luaL_addstring(&b, buf);

   if (loader->parser.problem_mark.line || loader->parser.problem_mark.column) {
      snprintf(buf, sizeof(buf), ", line: %d, column: %d\n",
         (int) loader->parser.problem_mark.line + 1,
         (int) loader->parser.problem_mark.column + 1);
      luaL_addstring(&b, buf);
   } else {
      luaL_addstring(&b, "\n");
   }

   if (loader->parser.context) {
      snprintf(buf, sizeof(buf), "%s at line: %d, column: %d\n",
         loader->parser.context,
         (int) loader->parser.context_mark.line + 1,
         (int) loader->parser.context_mark.column + 1);
      luaL_addstring(&b, buf);
   }

   luaL_pushresult(&b);
}

static inline void delete_event(struct lua_yaml_loader *loader) {
   if (loader->validevent) {
      yaml_event_delete(&loader->event);
      loader->validevent = 0;
   }
}

static inline int do_parse(struct lua_yaml_loader *loader) {
   delete_event(loader);
   if (yaml_parser_parse(&loader->parser, &loader->event) != 1) {
      generate_error_message(loader);
      loader->error = 1;
      return 0;
   }

   loader->validevent = 1;
   return 1;
}

static int load_node(struct lua_yaml_loader *loader);

static void handle_anchor(struct lua_yaml_loader *loader) {
   const char *anchor = (char *)loader->event.data.scalar.anchor;
   if (!anchor)
      return;

   lua_pushstring(loader->L, anchor);
   lua_pushvalue(loader->L, -2);
   lua_rawset(loader->L, loader->anchortable_index);
}

static void load_map(struct lua_yaml_loader *loader) {
   lua_createtable(loader->L, 0, 5);
   if (loader->cfg->decode_save_metatables)
      luaL_setmaphint(loader->L, -1);

   handle_anchor(loader);
   while (1) {
      int r;
      /* load key */
      if (load_node(loader) == 0 || loader->error)
         return;

      /* load value */
      r = load_node(loader);
      if (loader->error)
         return;
      if (r != 1)
         RETURN_ERRMSG(loader, "unanticipated END event");
      lua_rawset(loader->L, -3);
   }
}

static void load_sequence(struct lua_yaml_loader *loader) {
   int index = 1;

   lua_createtable(loader->L, 5, 0);
   if (loader->cfg->decode_save_metatables)
      luaL_setarrayhint(loader->L, -1);

   handle_anchor(loader);
   while (load_node(loader) == 1 && !loader->error)
      lua_rawseti(loader->L, -2, index++);
}

static void load_scalar(struct lua_yaml_loader *loader) {
   const char *str = (char *)loader->event.data.scalar.value;
   unsigned int length = loader->event.data.scalar.length;
   const char *tag = (char *)loader->event.data.scalar.tag;

   if (tag && !strncmp(tag, LUAYAML_TAG_PREFIX, sizeof(LUAYAML_TAG_PREFIX) - 1)) {
      tag += sizeof(LUAYAML_TAG_PREFIX) - 1;

      if (!strcmp(tag, "str")) {
         lua_pushlstring(loader->L, str, length);
         return;
      } else if (!strcmp(tag, "int")) {
         lua_pushinteger(loader->L, strtol(str, NULL, 10));
         return;
      } else if (!strcmp(tag, "float")) {
         double dval = fpconv_strtod(str, NULL);
         luaL_checkfinite(loader->L, loader->cfg, dval);
         lua_pushnumber(loader->L, dval);
         return;
      } else if (!strcmp(tag, "bool")) {
         lua_pushboolean(loader->L, !strcmp(str, "true") || !strcmp(str, "yes"));
         return;
      } else if (!strcmp(tag, "binary")) {
         frombase64(loader->L, (const unsigned char *)str, length);
         return;
      }
   }

   if (loader->event.data.scalar.style == YAML_PLAIN_SCALAR_STYLE) {
      if (!strcmp(str, "~")) {
         luaL_pushnull(loader->L);
         return;
      } else if (!strcmp(str, "true") || !strcmp(str, "yes")) {
         lua_pushboolean(loader->L, 1);
         return;
      } else if (!strcmp(str, "false") || !strcmp(str, "no")) {
         lua_pushboolean(loader->L, 0);
         return;
      } else if (!strcmp(str, "null")) {
         luaL_pushnull(loader->L);
         return;
      } else if (!length) {
         lua_pushliteral(loader->L, "");
         return;
      }

      /* plain scalar and Lua can convert it to a number?  make it so... */
      char *endptr = NULL;
      long long ival = strtoll(str, &endptr, 10);
      if (endptr == str + length && ival != LLONG_MAX) {
         luaL_pushint64(loader->L, ival);
         return;
      }
      unsigned long long uval = strtoull(str, &endptr, 10);
      if (endptr == str + length) {
         luaL_pushuint64(loader->L, uval);
         return;
      }
      double dval = fpconv_strtod(str, &endptr);
      if (endptr == str + length) {
         luaL_checkfinite(loader->L, loader->cfg, dval);
         lua_pushnumber(loader->L, dval);
         return;
      }
   }

   lua_pushlstring(loader->L, str, length);

   handle_anchor(loader);
}

static void load_alias(struct lua_yaml_loader *loader) {
   char *anchor = (char *)loader->event.data.alias.anchor;
   lua_pushstring(loader->L, anchor);
   lua_rawget(loader->L, loader->anchortable_index);
   if (lua_isnil(loader->L, -1)) {
      char buf[256];
      snprintf(buf, sizeof(buf), "invalid reference: %s", anchor);
      RETURN_ERRMSG(loader, buf);
   }
}

static int load_node(struct lua_yaml_loader *loader) {
   if (!do_parse(loader))
      return -1;

   switch (loader->event.type) {
      case YAML_DOCUMENT_END_EVENT:
      case YAML_MAPPING_END_EVENT:
      case YAML_SEQUENCE_END_EVENT:
         return 0;

      case YAML_MAPPING_START_EVENT:
         load_map(loader);
         return 1;

      case YAML_SEQUENCE_START_EVENT:
         load_sequence(loader);
         return 1;

      case YAML_SCALAR_EVENT:
         load_scalar(loader);
         return 1;

      case YAML_ALIAS_EVENT:
         load_alias(loader);
         return 1;

      case YAML_NO_EVENT:
         lua_pushliteral(loader->L, "libyaml returned YAML_NO_EVENT");
         loader->error = 1;
         return -1;

      default:
         lua_pushliteral(loader->L, "invalid event");
         loader->error = 1;
         return -1;
   }
}

static void load(struct lua_yaml_loader *loader) {
   if (!do_parse(loader))
      return;

   if (loader->event.type != YAML_STREAM_START_EVENT)
      RETURN_ERRMSG(loader, "expected STREAM_START_EVENT");

   while (1) {
      if (!do_parse(loader))
         return;

      if (loader->event.type == YAML_STREAM_END_EVENT)
         return;

      loader->document_count++;
      if (load_node(loader) != 1)
         RETURN_ERRMSG(loader, "unexpected END event");
      if (loader->error)
         return;

      if (!do_parse(loader))
         return;
      if (loader->event.type != YAML_DOCUMENT_END_EVENT)
         RETURN_ERRMSG(loader, "expected DOCUMENT_END_EVENT");

      /* reset anchor table */
      lua_newtable(loader->L);
      lua_replace(loader->L, loader->anchortable_index);
   }
}

static int l_load(lua_State *L) {
   struct lua_yaml_loader loader;

   luaL_argcheck(L, lua_isstring(L, 1), 1, "must provide a string argument");

   loader.L = L;
   loader.cfg = luaL_checkserializer(L);
   loader.validevent = 0;
   loader.error = 0;
   loader.document_count = 0;

   /* create table used to track anchors */
   lua_newtable(L);
   loader.anchortable_index = lua_gettop(L);

   yaml_parser_initialize(&loader.parser);
   yaml_parser_set_input_string(&loader.parser,
      (const unsigned char *)lua_tostring(L, 1), lua_strlen(L, 1));
   load(&loader);

   delete_event(&loader);
   yaml_parser_delete(&loader.parser);

   if (loader.error)
      lua_error(L);

   return loader.document_count;
}

static int dump_node(struct lua_yaml_dumper *dumper);

/*
  Copyright (c) 2013 Palard Julien. All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
  OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
  SUCH DAMAGE.

  Check if the given unsigned char * is a valid utf-8 sequence.

  Return value :
  If the string is valid utf-8, 1 is returned.
  Else, 0 is returned.

  Valid utf-8 sequences look like this :
  0xxxxxxx
  110xxxxx 10xxxxxx
  1110xxxx 10xxxxxx 10xxxxxx
  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
  1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
*/
static int is_utf8(const char *data, size_t len)
{
    const unsigned char *str = (const unsigned char *) data;
    size_t i = 0;
    size_t continuation_bytes = 0;

    while (i < len)
    {
        if (str[i] <= 0x7F)
            continuation_bytes = 0;
        else if (str[i] >= 0xC0 /*11000000*/ && str[i] <= 0xDF /*11011111*/)
            continuation_bytes = 1;
        else if (str[i] >= 0xE0 /*11100000*/ && str[i] <= 0xEF /*11101111*/)
            continuation_bytes = 2;
        else if (str[i] >= 0xF0 /*11110000*/ && str[i] <= 0xF4 /* Cause of RFC 3629 */)
            continuation_bytes = 3;
        else
            return 0;
        i += 1;
        while (i < len && continuation_bytes > 0
               && str[i] >= 0x80
               && str[i] <= 0xBF)
        {
            i += 1;
            continuation_bytes -= 1;
        }
        if (continuation_bytes != 0)
            return 0;
    }
    return 1;
}

static yaml_char_t *get_yaml_anchor(struct lua_yaml_dumper *dumper) {
   const char *s = "";
   lua_pushvalue(dumper->L, -1);
   lua_rawget(dumper->L, dumper->anchortable_index);
   if (!lua_toboolean(dumper->L, -1)) {
      lua_pop(dumper->L, 1);
      return NULL;
   }

   if (lua_isboolean(dumper->L, -1)) {
      /* this element is referenced more than once but has not been named */
      char buf[32];
      snprintf(buf, sizeof(buf), "%u", dumper->anchor_number++);
      lua_pop(dumper->L, 1);
      lua_pushvalue(dumper->L, -1);
      lua_pushstring(dumper->L, buf);
      s = lua_tostring(dumper->L, -1);
      lua_rawset(dumper->L, dumper->anchortable_index);
   } else {
      /* this is an aliased element */
      yaml_event_t ev;
      yaml_alias_event_initialize(&ev, (yaml_char_t *)lua_tostring(dumper->L, -1));
      yaml_emitter_emit(&dumper->emitter, &ev);
      lua_pop(dumper->L, 1);
   }
   return (yaml_char_t *)s;
}

static int dump_table(struct lua_yaml_dumper *dumper, struct luaL_field *field){
   yaml_event_t ev;
   yaml_char_t *anchor = get_yaml_anchor(dumper);

   if (anchor && !*anchor) return 1;

   yaml_mapping_style_t yaml_style = (field->compact)
      ? (YAML_FLOW_MAPPING_STYLE) : YAML_BLOCK_MAPPING_STYLE;
   yaml_mapping_start_event_initialize(&ev, anchor, NULL, 0, yaml_style);
   yaml_emitter_emit(&dumper->emitter, &ev);

   lua_pushnil(dumper->L);
   while (lua_next(dumper->L, -2)) {
      lua_pushvalue(dumper->L, -2); /* push copy of key on top of stack */
      if (!dump_node(dumper) || dumper->error)
         return 0;
      lua_pop(dumper->L, 1); /* pop copy of key */
      if (!dump_node(dumper) || dumper->error)
         return 0;
      lua_pop(dumper->L, 1);
   }

   yaml_mapping_end_event_initialize(&ev);
   yaml_emitter_emit(&dumper->emitter, &ev);
   return 1;
}

static int dump_array(struct lua_yaml_dumper *dumper, struct luaL_field *field){
   int i;
   yaml_event_t ev;
   yaml_char_t *anchor = get_yaml_anchor(dumper);

   if (anchor && !*anchor)
      return 1;

   yaml_sequence_style_t yaml_style = (field->compact)
      ? (YAML_FLOW_SEQUENCE_STYLE) : YAML_BLOCK_SEQUENCE_STYLE;
   yaml_sequence_start_event_initialize(&ev, anchor, NULL, 0, yaml_style);
   yaml_emitter_emit(&dumper->emitter, &ev);

   for (i = 0; i < field->size; i++) {
      lua_rawgeti(dumper->L, -1, i + 1);
      if (!dump_node(dumper) || dumper->error)
         return 0;
      lua_pop(dumper->L, 1);
   }

   yaml_sequence_end_event_initialize(&ev);
   yaml_emitter_emit(&dumper->emitter, &ev);

   return 1;
}

static int dump_null(struct lua_yaml_dumper *dumper) {
   yaml_event_t ev;
   yaml_scalar_event_initialize(&ev, NULL, NULL,
      (unsigned char *)"null", 4, 1, 1, YAML_PLAIN_SCALAR_STYLE);
   return yaml_emitter_emit(&dumper->emitter, &ev);
}

static int dump_node(struct lua_yaml_dumper *dumper)
{
   size_t len;
   const char *str = NULL;
   yaml_char_t *tag = NULL;
   yaml_event_t ev;
   yaml_event_t *evp;
   yaml_scalar_style_t style = YAML_PLAIN_SCALAR_STYLE;
   int is_binary = 0;
   char buf[FPCONV_G_FMT_BUFSIZE];
   struct luaL_field field;

   int top = lua_gettop(dumper->L);
   luaL_checkfield(dumper->L, dumper->cfg, top, &field);
   switch(field.type) {
   case MP_UINT:
      snprintf(buf, sizeof(buf) - 1, "%" PRIu64, field.ival);
      buf[sizeof(buf) - 1] = 0;
      str = buf;
      len = strlen(buf);
      break;
   case MP_INT:
      snprintf(buf, sizeof(buf) - 1, "%" PRIi64, field.ival);
      buf[sizeof(buf) - 1] = 0;
      str = buf;
      len = strlen(buf);
      break;
   case MP_FLOAT:
      fpconv_g_fmt(buf, field.fval, dumper->cfg->encode_number_precision);
      str = buf;
      len = strlen(buf);
      break;
   case MP_DOUBLE:
      fpconv_g_fmt(buf, field.dval, dumper->cfg->encode_number_precision);
      str = buf;
      len = strlen(buf);
      break;
   case MP_ARRAY:
      return dump_array(dumper, &field);
   case MP_MAP:
      return dump_table(dumper, &field);
   case MP_BIN:
   case MP_STR:
      str = lua_tolstring(dumper->L, -1, &len);
      if (field.type == MP_BIN || !is_utf8(str, len)) {
         is_binary = 1;
         tobase64(dumper->L, -1);
         str = lua_tolstring(dumper->L, -1, &len);
         tag = (yaml_char_t *) LUAYAML_TAG_PREFIX "binary";
         break;
      }

      /*
       * Always quote strings in FLOW SEQUENCE
       * Flow: [1, 'a', 'testing']
       * Block:
       * - 1
       * - a
       * - testing
       */
      if (dumper->emitter.flow_level > 0) {
         style = YAML_SINGLE_QUOTED_SCALAR_STYLE;
         break;
      }

      for (evp = dumper->emitter.events.head;
         evp != dumper->emitter.events.tail; evp++) {
         if (evp->type == YAML_SEQUENCE_START_EVENT &&
             evp->data.sequence_start.style == YAML_FLOW_SEQUENCE_STYLE) {
            style = YAML_SINGLE_QUOTED_SCALAR_STYLE;
            goto strbreak;
         }
      }

      if (len <= 5 && (!strcmp(str, "true")
          || !strcmp(str, "false")
          || !strcmp(str, "~")
          || !strcmp(str, "null"))) {
          style = YAML_SINGLE_QUOTED_SCALAR_STYLE;
      } else if (lua_isnumber(dumper->L, -1)) {
         /* string is convertible to number, quote it to preserve type */
         style = YAML_SINGLE_QUOTED_SCALAR_STYLE;
      }
      strbreak:
      break;
   case MP_BOOL:
      if (field.bval) {
         str = "true";
         len = 4;
      } else {
         str = "false";
         len = 5;
      }
      break;
   case MP_NIL:
      return dump_null(dumper);
   case MP_EXT:
      assert(false);
      break;
    }

   yaml_scalar_event_initialize(&ev, NULL, tag, (unsigned char *)str, len,
                                !is_binary, !is_binary, style);
   if (is_binary)
      lua_pop(dumper->L, 1);

   return yaml_emitter_emit(&dumper->emitter, &ev);
}

static void dump_document(struct lua_yaml_dumper *dumper) {
   yaml_event_t ev;

   yaml_document_start_event_initialize(&ev, NULL, NULL, NULL, 0);
   yaml_emitter_emit(&dumper->emitter, &ev);

   if (!dump_node(dumper) || dumper->error)
      return;

   yaml_document_end_event_initialize(&ev, 0);
   yaml_emitter_emit(&dumper->emitter, &ev);
}

static int append_output(void *arg, unsigned char *buf, size_t len) {
   struct lua_yaml_dumper *dumper = (struct lua_yaml_dumper *)arg;
   luaL_addlstring(&dumper->yamlbuf, (char *)buf, len);
   return 1;
}

static void find_references(struct lua_yaml_dumper *dumper) {
   int newval = -1, type = lua_type(dumper->L, -1);
   if (type != LUA_TTABLE)
      return;

   lua_pushvalue(dumper->L, -1); /* push copy of table */
   lua_rawget(dumper->L, dumper->anchortable_index);
   if (lua_isnil(dumper->L, -1))
      newval = 0;
   else if (!lua_toboolean(dumper->L, -1))
      newval = 1;
   lua_pop(dumper->L, 1);
   if (newval != -1) {
      lua_pushvalue(dumper->L, -1);
      lua_pushboolean(dumper->L, newval);
      lua_rawset(dumper->L, dumper->anchortable_index);
   }
   if (newval)
      return;

   /* recursively process other table values */
   lua_pushnil(dumper->L);
   while (lua_next(dumper->L, -2) != 0) {
      find_references(dumper); /* find references on value */
      lua_pop(dumper->L, 1);
      find_references(dumper); /* find references on key */
   }
}

static int l_dump(lua_State *L) {
   struct lua_yaml_dumper dumper;
   int i, argcount = lua_gettop(L);
   yaml_event_t ev;

   dumper.L = L;
   dumper.cfg = luaL_checkserializer(L);
   dumper.error = 0;
   /* create thread to use for YAML buffer */
   dumper.outputL = lua_newthread(L);
   luaL_buffinit(dumper.outputL, &dumper.yamlbuf);

   yaml_emitter_initialize(&dumper.emitter);
   yaml_emitter_set_unicode(&dumper.emitter, 1);
   yaml_emitter_set_indent(&dumper.emitter, 2);
   yaml_emitter_set_width(&dumper.emitter, 2);
   yaml_emitter_set_break(&dumper.emitter, YAML_LN_BREAK);
   yaml_emitter_set_output(&dumper.emitter, &append_output, &dumper);

   yaml_stream_start_event_initialize(&ev, YAML_UTF8_ENCODING);
   yaml_emitter_emit(&dumper.emitter, &ev);

   for (i = 0; i < argcount; i++) {
      lua_newtable(L);
      dumper.anchortable_index = lua_gettop(L);
      dumper.anchor_number = 0;
      lua_pushvalue(L, i + 1); /* push copy of arg we're processing */
      find_references(&dumper);
      dump_document(&dumper);
      if (dumper.error)
         break;
      lua_pop(L, 2); /* pop copied arg and anchor table */
   }

   yaml_stream_end_event_initialize(&ev);
   yaml_emitter_emit(&dumper.emitter, &ev);

   yaml_emitter_flush(&dumper.emitter);
   yaml_emitter_delete(&dumper.emitter);

   /* finalize and push YAML buffer */
   luaL_pushresult(&dumper.yamlbuf);

   if (dumper.error)
      lua_error(L);

   /* move buffer to original thread */
   lua_xmove(dumper.outputL, L, 1);
   return 1;
}

static int
l_new(lua_State *L);

const luaL_reg yamllib[] = {
   { "encode", l_dump },
   { "decode", l_load },
   { "new",    l_new },
   { NULL, NULL}
};

static int
l_new(lua_State *L)
{
   struct luaL_serializer *s = luaL_newserializer(L, NULL, yamllib);
   s->has_compact = 1;
   return 1;
}

int
luaopen_yaml(lua_State *L) {
   struct luaL_serializer *s = luaL_newserializer(L, "yaml", yamllib);
   s->has_compact = 1;
   return 1;
}

/* vim: et sw=3 ts=3 sts=3:
*/
