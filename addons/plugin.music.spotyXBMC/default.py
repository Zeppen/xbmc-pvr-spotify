import xbmcplugin,xbmcgui,xbmcaddon

# plugin constants
__version__ = "0.0.1"
__plugin__ = "spotyXBMC-" + __version__
__author__ = "David Erenger"
__url__ = "https://github.com/akezeke/xbmc"
__settings__ = xbmcaddon.Addon(id='plugin.music.spotyXBMC')
__language__ = __settings__.getLocalizedString

def startPlugin():
	if (__settings__.getSetting("enable") == "false"):
		__settings__.openSettings()
	else:
		dialog = xbmcgui.Dialog()
		dialog.ok("spotyXBMC2", "Running the addon will not lead you to spotify music", "instead check out the regular secions in the", "music library!")

	#TODO why is this leading to a list without playlists and search? Find a better way to do this
	xbmc.executebuiltin('XBMC.ActivateWindow(music,musicdb://,Return)')		

startPlugin()
