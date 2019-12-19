local route = ""
local header =
"HTTP/1.1 {{ .responseCode }} {{ .responseStatus }}\r\n" ..
"Content-Length: {{ .length }}\r\n" ..
"{{ .addedHeader }}" ..
"\r\n"
local template = ""
local html = ""

for _, r in string.gmatch(__buffer, '(%s+)(/[A-Za-z0-9%.%%]*)') do
    route = r
    break
end

if (#route > 1) then
    files = { "/favicon.ico", "/echarts.min.js", "/chartRender.js" }
    flag = (function()
        for _, v in pairs(files) do
            if (route == v) then
                return true
            end
        end
        return false
    end)()
    if (flag == false) then
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
        header = string.gsub(header, "{{(%s*).responseCode(%s*)}}", "200")
        header = string.gsub(header, "{{(%s*).responseStatus(%s*)}}", "OK")
        header = string.gsub(header, "{{(%s*).addedHeader(%s*)}}", "")
    else
        __sendFile(__conn, "." .. route)
        return
    end
else
    code = ""
    for _, v in string.gmatch(__buffer, '( /%?code=)(%w+)') do
        code = v
        break
    end
    if (code == "") then
        template = (function()
            local file = io.open("./index.html", "rb")
            local len = file:seek("end")
            file:seek("set")
            local data = file:read(len)
            io.close(file)
            return data
        end)()
        html = template
        header = string.gsub(header, "{{(%s*).responseCode(%s*)}}", "200")
        header = string.gsub(header, "{{(%s*).responseStatus(%s*)}}", "OK")
        header = string.gsub(header, "{{(%s*).addedHeader(%s*)}}", "")
    else
        template = (function()
            local file = io.open("./302.html", "rb")
            local len = file:seek("end")
            file:seek("set")
            local data = file:read(len)
            io.close(file)
            return data
        end)()
        html = template
        header = string.gsub(header, "{{(%s*).responseCode(%s*)}}", "302")
        header = string.gsub(header, "{{(%s*).responseStatus(%s*)}}", "Found")
        header = string.gsub(header, "{{(%s*).addedHeader(%s*)}}", "Location: /" .. code .. "\r\n")
    end
end
header = string.gsub(header, "{{(%s*).length(%s*)}}", #html)
__sendHttpResponse(__conn, header .. html)
