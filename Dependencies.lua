function download_progress(total, current)
    local ratio = current / total;
    ratio = math.min(math.max(ratio, 0), 1);
    local percent = math.floor(ratio * 100);
    print("Download progress (" .. percent .. "%/100%)")
end

glfw_dir = "Vendor/Sources/glfw-3.4.bin.WIN64"
spdlog_dir = "Vendor/Sources/spdlog-1.17.0"
stb_dir = "Vendor/Sources/stb"
imgui_commit = "6029ee3789a2b7898f6423ec0c88cc4e5425f5a9" -- imgui docking branch, pinned for docking support
imgui_dir = "Vendor/Sources/imgui-" .. imgui_commit

function check_spdlog()
    print("Checking for spdlog...")
    print(os.getcwd())
    os.chdir("Vendor")
    if(os.isdir("Sources") == false) then
        os.mkdir("Sources")
    end
    os.chdir("Sources")
    if(os.isdir("spdlog-1.17.0") == false) then
        if(not os.isfile("spdlog.zip")) then
            print("spdlog not found, downloading from https://github.com/gabime/spdlog/archive/refs/tags/v1.17.0.zip")
            local result_str, response_code = http.download("https://github.com/gabime/spdlog/archive/refs/tags/v1.17.0.zip", "spdlog.zip", {
                progress = download_progress,
                headers = { "From: Premake", "Referer: Premake" }
            })
        end
        print("Unzipping to " ..  os.getcwd())
        zip.extract("spdlog.zip", os.getcwd())
        os.remove("spdlog.zip")
    end
    os.chdir("../../")
end

function check_glfw()
    print("Checking for glfw...")
    print(os.getcwd())
    os.chdir("Vendor")
    if(os.isdir("Sources") == false) then
        os.mkdir("Sources")
    end
    os.chdir("Sources")
    if(os.isdir("glfw-3.4.bin.WIN64") == false) then
        if(not os.isfile("glfw.zip")) then
            print("glfw not found, downloading from https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.bin.WIN64.zip")
            local result_str, response_code = http.download("https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.bin.WIN64.zip", "glfw.zip", {
                progress = download_progress,
                headers = { "From: Premake", "Referer: Premake" }
            })
        end
        print("Unzipping to " ..  os.getcwd())
        zip.extract("glfw.zip", os.getcwd())
        os.remove("glfw.zip")
    end
    os.chdir("../../")
end

function check_stb_image()
    print("Checking for stb_image...")
    print(os.getcwd())
    os.chdir("Vendor")
    if(os.isdir("Sources") == false) then
        os.mkdir("Sources")
    end
    os.chdir("Sources")
    if(os.isdir("stb") == false) then
        os.mkdir("stb")
    end
    if(not os.isfile("stb/stb_image.h")) then
        print("stb_image.h not found, downloading from https://raw.githubusercontent.com/nothings/stb/master/stb_image.h")
        local result_str, response_code = http.download("https://raw.githubusercontent.com/nothings/stb/master/stb_image.h", "stb/stb_image.h", {
            progress = download_progress,
            headers = { "From: Premake", "Referer: Premake" }
        })
    end
    os.chdir("../../")
end

function check_imgui()
    print("Checking for imgui...")
    print(os.getcwd())
    os.chdir("Vendor")
    if(os.isdir("Sources") == false) then
        os.mkdir("Sources")
    end
    os.chdir("Sources")
    if(os.isdir("imgui-" .. imgui_commit) == false) then
        if(not os.isfile("imgui.zip")) then
            print("imgui not found, downloading from https://github.com/ocornut/imgui/archive/" .. imgui_commit .. ".zip")
            local result_str, response_code = http.download("https://github.com/ocornut/imgui/archive/" .. imgui_commit .. ".zip", "imgui.zip", {
                progress = download_progress,
                headers = { "From: Premake", "Referer: Premake" }
            })
        end
        print("Unzipping to " ..  os.getcwd())
        zip.extract("imgui.zip", os.getcwd())
        os.remove("imgui.zip")
    end
    os.chdir("../../")
end

function build_externals()
     print("Checking external dependencies...")
     check_spdlog()
     check_stb_image()
     check_imgui()
     filter "system:windows"
        check_glfw()
end

build_externals()
