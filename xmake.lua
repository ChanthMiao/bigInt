
-- add modes: debug and release
add_rules("mode.debug", "mode.release")
set_languages("c11", "cxx17")
-- set warning all as error
set_warnings("all")
-- set optimization: none, faster, fastest, smallest
set_optimize("fastest")

option("dynamic")
    set_default(false)
    set_showmenu(true)
    set_category("option")
    set_description("Enable the dynamic export.")
option_end()
-- add support for the toolchain of visual studio
if is_plat("windows") then
    add_cflags("/utf-8", "/std:c++latest")
    add_defines("_CRT_SECURE_NO_WARNINGS")
end

-- add target
target("bigInt")
    -- set kind
    if has_config("dynamic") then
        set_kind("shared")
        if is_plat("linux") then
            add_cflags("-fPIC")
        end
    else
        set_kind("static")
    end
    -- add files
    add_files("src/lib/*.c")

-- add target
target("demo")
    add_deps("bigInt")
    -- set kind
    set_kind("binary")
    add_links("bigInt")
    add_includedirs("src/lib")
    -- add files
    add_files("src/demo/*.c")


-- FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add macro defination
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "cxx11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox", "z", "pthread")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--
-- 7. If you want to known more usage about xmake, please see http://xmake.io/#/home
--
