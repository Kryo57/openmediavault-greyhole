/**
 * vim: tabstop=4
 *
 * @license http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author Stephane Bocquet <stephane_bocquet@hotmail.com>
 * @author Marcel Beck <marcel.beck@mbeck.org>
 * @copyright Copyright (c) 2011 Stephane Bocquet
 * @copyright Copyright (c) 2011 Marcel Beck
 * @version $Id: greyhole.js 12 2011-11-07 18:52:10Z
 *          stephane_bocquet@hotmail.com $
 *
 * This file is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or any later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this file. If not, see <http://www.gnu.org/licenses/>.
 */
// require("js/omv/NavigationPanel.js")
// require("js/omv/data/DataProxy.js")
// require("js/omv/FormPanelExt.js")
// require("js/omv/form/SharedFolderComboBox.js")
// require("js/omv/form/plugins/FieldInfo.js")

// require("js/omv/module/greyhole/NavigationPanel.js")

Ext.ns("OMV.Module.Storage.Greyhole.Admin");

/**
 * General settings - First tab
 */
OMV.Module.Storage.Greyhole.Admin.SettingsPanel = function(config) {
	var initialConfig = {
		title: "Settings",
		rpcService: "Greyhole",
		rpcGetMethod: "getSettings",
		rpcSetMethod: "setSettings"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.Greyhole.Admin.SettingsPanel.superclass.constructor.call(this, initialConfig);
};

Ext.extend(OMV.Module.Storage.Greyhole.Admin.SettingsPanel, OMV.FormPanelExt, {
	initComponent : function() {
		OMV.Module.Storage.Greyhole.Admin.SettingsPanel.superclass.initComponent.apply(this, arguments);
	},

	getFormItems : function() {
		return [{
			xtype: "fieldset",
			title: "General settings",
			defaults: {
				labelSeparator: ""
			},
			items: [{
					xtype: "checkbox",
					name: "enable",
					fieldLabel: "Enable",
					checked: false,
					inputValue: 1
				},{
					xtype: "textfield",
					name: "email_to",
					fieldLabel: "Email",
					allowBlank: false,
					vtype: "email",
					infoText: "Will receive email reports for daily fsck, or when all drives are out of available space."
				},{
					xtype: "combo",
					name: "log_level",
					hiddenName: "log_level",
					fieldLabel: "Log level",
					mode: "local",
					store: new Ext.data.SimpleStore({
							fields: [ "value","text" ],
							data: [
									[ "ERROR","Error" ],
									[ "WARN","Warn" ],
									[ "INFO","Info" ],
									[ "DEBUG","Debug" ]
							]
					}),
					displayField: "text",
					valueField: "value",
					allowBlank: false,
					editable: false,
					triggerAction: "all",
					value: "INFO"
				},{
					xtype: "checkbox",
					name: "delete_moves_to_trash",
					fieldLabel: "Trash deletes",
					checked: true,
					inputValue: 1
			},{
					xtype: "checkbox",
					name: "balance_modified_files",
					fieldLabel: "Balance modified",
					checked: false,
					inputValue: 1
			},{
					xtype: "numberfield",
					name: "df_cache_time",
					fieldLabel: "DF Cache Time",
					inputValue: 15
			},{
					xtype: "checkbox",
					name: "log_memory_usage",
					fieldLabel: "Log Memory Usage",
					checked: false,
					inputValue: 1
			},{
					xtype: "checkbox",
					name: "check_for_open_files",
					fieldLabel: "Check for open Files",
					checked: false,
					inputValue: 1
			}]
		}];
	}
});

OMV.NavigationPanelMgr.registerPanel("storage", "greyhole", {
	cls: OMV.Module.Storage.Greyhole.Admin.SettingsPanel,
	position : 10,
	title : "Settings"
});
