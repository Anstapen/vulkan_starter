function download_progress(total, current)
    local ratio = current / total;
    ratio = math.min(math.max(ratio, 0), 1);
    local percent = math.floor(ratio * 100);
    print("Download progress (" .. percent .. "%/100%)")
end


function check_glm()
    print("Checking for GLM...")
    print(os.getcwd())
    os.chdir("Vendor")
    if(os.isdir("Sources") == false) then
        os.mkdir("Sources")
    end
    os.chdir("Sources")
    if(os.isdir("glm-master") == false) then
        if(not os.isfile("glm-master.zip")) then
            print("GLM not found, downloading from https://github.com/g-truc/glm/archive/refs/heads/master.zip")
            local result_str, response_code = http.download("https://github.com/g-truc/glm/archive/refs/heads/master.zip", "glm-master.zip", {
                progress = download_progress,
                headers = { "From: Premake", "Referer: Premake" }
            })
        end
        print("Unzipping to " ..  os.getcwd())
        zip.extract("glm-master.zip", os.getcwd())
        os.remove("glm-master.zip")
    end
    os.chdir("../../")
end

function build_externals()
     print("Checking external dependencies...")
     check_glm()
end

build_externals()
