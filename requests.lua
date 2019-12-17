local route = ""
local header =
"HTTP/1.1 200 OK\r\n" ..
"Content-Length: {{ .length }}" ..
"\r\n" ..
"\r\n"
local template = ""
local html = ""

for _, r in string.gmatch(__buffer, '(%s+)(/[A-Za-z0-9%.%%]*)') do
    route = r
    break
end

if (#route > 1) then
    if (route ~= "/echarts.min.js") then
        local symbol = string.sub(route, 2, #route)
        local json = __sendRequest("https://xueqiu.com/cubes/nav_daily/all.json?cube_symbol=" .. symbol)
        template = (function()
            local file = io.open("./template.html", "rb")
            local len = file:seek("end")
            file:seek("set")
            local data = file:read(len)
            io.close(file)
            return data
        end)()
        html = template
        html = string.gsub(html, "{{(%s*).dataString(%s*)}}", json)
    else
        local f = (function()
            local file = io.open("./echarts.min.js", "rb")
            local len = file:seek("end")
            file:seek("set")
            local data = file:read(len)
            io.close(file)
            return data
        end)()
        html = f
    end
else
    template = (function()
        local file = io.open("./index.html", "rb")
        local len = file:seek("end")
        file:seek("set")
        local data = file:read(len)
        io.close(file)
        return data
    end)()
    html = template
end
header = string.gsub(header, "{{(%s*).length(%s*)}}", #html)
__sendHttpResponse(__conn, header .. html)
