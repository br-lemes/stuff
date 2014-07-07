
cfg1 = "config"
cfg2 = "simpler/linux/.config"

table1 = { }
table2 = { }
table3 = { }

f = io.open(cfg1)
for line in f:lines() do
	local i = line:match('CONFIG_[A-Z_]*')
	if i then table1[i] = line end
end
f:close()

f = io.open(cfg2)
for line in f:lines() do
	local i = line:match('CONFIG_[A-Z_]*')
	if i then table2[i] = line end
end
f:close()

for k,v in pairs(table1) do
	if v ~= table2[k] then
		table3[k] = true
	end
end

for k,v in pairs(table2) do
	if v ~= table1[k] then
		table3[k] = true
	end
end

for k,v in pairs(table3) do
	if table1[k] then print("-" .. table1[k]) end
	if table2[k] then print("+" .. table2[k]) end
end
