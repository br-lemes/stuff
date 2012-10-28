-- Copyright (c) 2012 Breno Ramalho Lemes <breno@br-lemes.net>
-- http://www.br-lemes.net/

-- Verifica o status da conexão de internet, o IP e o tempo conectado do meu
-- roteador wireless Smart Lan APRIO150. Mostra a informação em um ícone na
-- área de notificação.
--
-- Histórico:
--
-- 27-04-2012 * Versão inicial (shell script)
-- 28-10-2012 * Versão IUP Lua
--
-- Licença: WTFPL
--
--            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
--                    Version 2, December 2004
--
-- Copyright (C) 2004 Sam Hocevar <sam@hocevar.net>
--
-- Everyone is permitted to copy and distribute verbatim or modified
-- copies of this license document, and changing it is allowed as long
-- as the name is changed.
--
--            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
--   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
--
--  0. You just DO WHAT THE FUCK YOU WANT TO.

require( "iuplua" )

if iup.GetGlobal("DRIVER") == "GTK" then
	yesimg = "yes.png"
	noimg = "no.png"
elseif iup.GetGlobal("DRIVER") == "Win32" then
	require("winpopen")
	yesimg = "yes.ico"
	noimg = "no.ico"
end

dg = iup.dialog{tray = "YES", traytip =  "Please wait...", trayimage = noimg}

timer = iup.timer{time=1000}
function timer:action_cb()
	pipe = io.popen("wget -qO- --timeout=1 http://192.168.1.254/cgi-bin/status/")
	source = pipe:read("*a")
	pipe:close()
	if source == "" then
		dg.traytip = "Please wait..."
	else
		status = source:match('name="WAN_LINK_STATUS" value="(.-)"')
		ipaddr = source:match('name="WAN_IP_ADDR" value="(.-)"')
		conn_time = source:match('name="INTERNET_CONN_TIME" value="(.-)"')

		conn_day = (conn_time - conn_time % 86400) / 86400
		conn_time = conn_time - conn_day * 86400
		conn_hr = (conn_time - conn_time % 3600) / 3600
		conn_time = conn_time - conn_hr * 3600
		conn_min = (conn_time - conn_time % 60) / 60
		conn_sec = conn_time - conn_min * 60

		dg.traytip = status .. '\n' .. ipaddr .. '\n' .. 
			string.format('%d dia(s) %d hora(s) %d minuto(s) %d segundo(s)',
			conn_day, conn_hr, conn_min, conn_sec)
	end
	if dg.traytip:find("Connected") then
		dg.trayimage = yesimg
	else
		dg.trayimage = noimg
	end
end
timer.run = "YES"

dg:show()
dg.hidetaskbar = "YES"
iup.MainLoop()
