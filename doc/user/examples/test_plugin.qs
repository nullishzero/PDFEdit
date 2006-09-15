//Test plugin

/** Function to install the menu/toolbar items that are used by the plugin */
function installPlugin() {
 createMenuItem('helpmenu','-','','');
 createMenuItem('helpmenu','plugintest','Menu item from test plugin','testplugin()','Ctrl+Alt+X','help.png');
 createMenuItem('MainToolbar','-','','');
 createMenuItem('MainToolbar','plugintest2','Toolbar item from test plugin','testplugin()','','help.png');
}

/** Function to invoke the pluigin functionality */
function testplugin() {
 message('This is a test plugin');
}

//Install the plugin
installPlugin();
