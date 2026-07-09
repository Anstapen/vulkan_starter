function download_progress(total, current)
    local ratio = current / total;
    ratio = math.min(math.max(ratio, 0), 1);
    local percent = math.floor(ratio * 100);
    print("Download progress (" .. percent .. "%/100%)")
end

glfw_dir = "Vendor/Sources/glfw-3.4.bin.WIN64"
spdlog_dir = "Vendor/Sources/spdlog-1.17.0"

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

function build_externals()
     print("Checking external dependencies...")
     check_spdlog()
     filter "system:windows"
        check_glfw()
end

build_externals()
