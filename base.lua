function getHttpRequestMethod(buffer)
    result = string.match(buffer, '(%w+)( )(/)')
    return result
end

function getHttpRequestHeaders(buffer)
    result = "Header:{"
    flag = true
    while (true) do
        t, _, _, _, _, _, k = string.find(buffer, '(\r\n)((.-)(:%s*)())', k)
        if (k == nil) then
            break
        end
        key = string.sub(buffer, t + 2, k - 3)
        f = string.find(buffer, '\r\n', k)
        space = " "
        if (flag) then
            space = ""
            flag = false
        end
        result = result .. space .. key .. ":[" .. string.sub(buffer, k, f - 1) .. "]"
    end
    result = result .. "}"
    return result
end

methodString = getHttpRequestMethod(__buffer)
headerString = getHttpRequestHeaders(__buffer)
pathString = (function()
    for _, r in string.gmatch(__buffer, '(%s+)(/[A-Za-z0-9%.%%]*)') do
        return r
    end
end)()
