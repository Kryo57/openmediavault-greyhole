/**
 * vim: tabstop=4
 *
 * @license http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author Stephane Bocquet <stephane_bocquet@hotmail.com>
 * @copyright Copyright (c) 2011 Ian Moore
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

Ext.ns("OMV.Module.Services");

// Register the menu.
OMV.NavigationPanelMgr.registerMenu("services", "greyhole", {
	text: "Greyhole",
	icon: "images/greyhole.png"
});

/**
 * General settings - First tab
 */
OMV.Module.Services.Greyhole.SettingsPanel = function(config) {
	var initialConfig = {
		rpcService: "Greyhole"
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Services.Greyhole.SettingsPanel.superclass.constructor.call(this, initialConfig);
};

Ext.extend(OMV.Module.Services.Greyhole.SettingsPanel, OMV.FormPanelExt, {
	initComponent : function() {
		OMV.Module.Services.Greyhole.SettingsPanel.superclass.initComponent.apply(this, arguments);
		this.on("load", this._updateFormFields, this);
		this.on("load", function(){
			var checked = this.findFormField("enable").checked;
		}, this);
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
					name: "email",
					fieldLabel: "Email",
					allowBlank: false,
					vtype: "email",
					infoText: "Will receive email reports for daily fsck, or when all drives are out of available space."
				},{
					xtype: "combo",
					name: "loglevel",
					hiddenName: "loglevel",
					fieldLabel: "Log level",
					mode: "local",
					store: new Ext.data.SimpleStore({
							fields: [ "value","text" ],
							data: [
									[ 1,"Minimum" ],
									[ 2,"Normal" ],
									[ 3,"Full" ],
									[ 10,"Debug" ]
							]
					}),
					displayField: "text",
					valueField: "value",
					allowBlank: false,
					editable: false,
					triggerAction: "all",
					value: 1
			}]
		}];
	},

	/**
	 * Private function to update the states of various form fields.
	 */
	_updateFormFields : function() {
		// Enabled/Disabled fields
		var field = this.findFormField("enable");
		var checked = field.checked;
	}
});

OMV.NavigationPanelMgr.registerPanel("services", "greyhole", {
	cls: OMV.Module.Services.Greyhole.SettingsPanel,
	position : 10,
	title : "Settings"
});


/**
 * Storage Pool list panel - Second tab TODO : Storage Pool grid with columns -
 * Disk - Min Free size - Other things ?
 */

/**
 * Share list panel - Third tab TODO : Share Pool grid with columns - Share name -
 * number of copies - other things
 */
