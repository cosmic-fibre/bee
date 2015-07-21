require 'formula'

class Bee < Formula
  homepage 'http://bee.org'

  depends_on 'cmake'    => :build
  depends_on "readline" => :build

  option 'with-debug', "Build Debug version"
  option 'with-tests', "Run Tests after building"

  stable do
    url 'https://github.com/bee/bee.git', :branch => "stable", :shallow => false
    depends_on 'e2fsprogs' => :recommended
    if build.with? 'tests'
      depends_on 'python-daemon' => [:python, "daemon",  :build]
      depends_on 'pyyaml'        => [:python, "yaml",    :build]
      depends_on 'pexpect'       => [:python, "pexpect", :build]
    end
    version "1.5"
  end

  devel do
    url 'https://github.com/bee/bee.git', :branch => "master", :shallow => false
    depends_on 'e2fsprogs' => :build
    if build.with? 'tests'
      depends_on 'python-daemon' => [:python, "daemon", :build]
      depends_on 'pyyaml'        => [:python, "yaml",   :build]
    end
    version "1.6"
  end

  def install
    args = []
    if build.with? 'debug'
      ENV.enable_warnings
      ENV.deparallelize
      args << "-DCMAKE_BUILD_TYPE=Debug"
      ohai "Building with Debug"
    else
      args << "-DCMAKE_BUILD_TYPE=Release"
      ohai "Building with Release"
    end
    args << "-DENABLE_CLIENT=True" if build.stable?
    args << "-DCMAKE_INSTALL_SYSCONFDIR=#{prefix}/etc"
    args << "-DCMAKE_INSTALL_LOCALSTATEDIR=#{prefix}/var"
    args << "-DREADLINE_ROOT=/usr/local/Cellar"
    args += std_cmake_args

    ohai "Preparing"
    version = `git -C #{cached_download} describe HEAD`

    File.open(buildpath/"VERSION", 'w') {|file| file.write(version)}

    ohai "Configuring:"
    system "cmake", ".", *args

    ohai "Building:"
    system "make"

    ohai "Installing:"
    system "make install"

    ohai "Installing man"
    man1.install 'doc/man/bee.1'
    if build.stable?
      man1.install 'doc/man/bee_db.1'
    end

    ohai "Installing config"
    if build.stable?
      inreplace prefix/"etc/bee.cfg", /^work_dir =.*/, "work_dir = #{prefix}/var/lib/bee"
    else
      doc.install "test/db/db.lua"
      inreplace doc/"db.lua" do |s|
          s.gsub!(/^os = require.*\n/    , '')
          s.gsub!(/os.getenv\("LISTEN"\)/, '3301')
          s.gsub!(/os.getenv\('ADMIN'\)/ , '3313')
          s.gsub!(/(rows_per_wal\s*=).*/ , '\1 500,')
          s.gsub!(/^}.*/                 , "\twork_dir\t\t\t= \"#{prefix}/var/lib/bee\",\n}")
      end
    end

    if build.with? 'tests'
        ohai "Testing Bee with internal test suite:"
        system "/usr/bin/env", "python", buildpath/"test/test-run.py", "--builddir", buildpath, "--vardir", buildpath/"test/var"
    end
  end

  test do
      system bin/"bee", "--version"
  end
end
