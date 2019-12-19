function getHttpRequestMethod(buffer)
    result = string.match(buffer, '(%w+)( )(/)')
    return result
end

function getHttpRequestHeaders(buffer)
    result = "Header:{"
    while (true) do
        t, _, _, _, _, _, k = string.find(buffer, '(\r\n)((.-)(:%s*)())', k)
        if (k == nil) then
            break
        end
        key = string.sub(buffer, t + 2, k - 3)
        f = string.find(buffer, '\r\n', k)
        result = result .. key .. ":[" .. string.sub(buffer, k, f - 1) .. "]"
    end
    result = result .. "}"
    return result
end

methodString = getHttpRequestMethod(__buffer)
headerString = getHttpRequestHeaders(__buffer)
