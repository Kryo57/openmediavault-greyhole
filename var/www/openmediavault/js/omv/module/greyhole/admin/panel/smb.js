/**
 * vim: tabstop=4
 *
 * @license http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author Stephane Bocquet <stephane_bocquet@hotmail.com>
 * @author Marcel Beck <marcel.beck@mbeck.org>
 * @copyright Copyright (c) 2011 Stephane Bocquet
 * @copyright Copyright (c) 2011 Marcel Beck
 * @version $Id: greyhole.js 12 2011-11-07 18:52:10Z
 *					stephane_bocquet@hotmail.com $
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

// require("js/omv/module/greyhole/panel/navigation.js")

// require("js/omv/module/greyhole/admin/dialog/smb.js")

Ext.ns("OMV.Module.Storage.Greyhole.Admin");

/**
 * @class OMV.Module.Storage.Greyhole.Admin.SMBPanel
 * @derived OMV.grid.TBarGridPanel
 * Display list of configured filesystems.
 */
OMV.Module.Storage.Greyhole.Admin.SMBPanel = function (config) {
	var initialConfig = {
		hidePagingToolbar:false,
		autoReload       :true,
		stateId          :"85f1cbf2-23d3-4960-a803-b7fc34d42235",
		colModel         :new Ext.grid.ColumnModel({
			columns:[
				{
					header   :"Shared folder",
					sortable :true,
					dataIndex:"name",
					id       :"name"
				},
				{
					header   :"Comment",
					sortable :true,
					dataIndex:"comment",
					id       :"comment"
				},
				{
					header   :"Files copies",
					sortable :true,
					dataIndex:"num_copies",
					id       :"num_copies",
					width    :20
				},
				{
					header   :"Stiky files",
					sortable :true,
					dataIndex:"stiky_files",
					id       :"stiky_files",
					renderer: OMV.util.Format.booleanRenderer(),
					width    :20
				},
				{
					header   :"Use Trash",
					sortable :true,
					dataIndex:"trash",
					id       :"trash",
					renderer: OMV.util.Format.booleanRenderer(),
					width    :20
				}
			]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.Greyhole.Admin.SMBPanel.superclass.constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.Greyhole.Admin.SMBPanel, OMV.grid.TBarGridPanel, {
	initComponent:function () {
		this.store = new OMV.data.Store({
			autoLoad  :true,
			remoteSort:false,
			proxy     :new OMV.data.DataProxy("Greyhole", "getSMBList"),
			reader    :new Ext.data.JsonReader({
				idProperty   :"uuid",
				totalProperty:"total",
				root         :"data",
				fields       :[
					{ name:"uuid" },
					{ name:"name" },
					{ name:"comment" },
					{ name:"num_copies" },
					{ name:"stiky_files" },
					{ name:"trash" },
				]
			})
		});
		OMV.Module.Storage.Greyhole.Admin.SMBPanel.superclass.initComponent.apply(this, arguments);
		// Register event handler
		// Reselect previous selected rows after the store has been
		// reloaded, e.g. to make sure toolbar is updated depending on
		// the latest row record values.
		this.getSelectionModel().previousSelections = [];
		this.store.on("beforeload", function (store, options) {
			var sm = this.getSelectionModel();
			var records = sm.getSelections();
			sm.previousSelections = [];
			Ext.each(records, function (record, index) {
				sm.previousSelections.push(record.get("uuid"));
			}, this);
		}, this);
		this.store.on("load", function (store, records, options) {
			var sm = this.getSelectionModel();
			var rows = [];
			if (Ext.isDefined(sm.previousSelections)) {
				for (var i = 0; i < sm.previousSelections.length; i++) {
					var index = store.findExact("uuid",
									sm.previousSelections[i]);
					if (index !== -1) {
						rows.push(index);
					}
				}
			}
			if (rows.length > 0) {
				sm.selectRows(rows);
			}
		}, this);
	},

	initToolbar:function () {
		var tbar = OMV.Module.Storage.Greyhole.Admin.SMBPanel.superclass.initToolbar.apply(this);
		return tbar;
	},

	cbAddBtnHdl:function () {
		var wnd = new OMV.Module.Storage.Greyhole.Admin.SMBDialog({
			uuid     :OMV.UUID_UNDEFINED,
			listeners:{
				submit:function () {
					this.doReload();
				},
				scope :this
			}
		});
		wnd.show();
	},

	cbEditBtnHdl:function () {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.Storage.Greyhole.Admin.SMBDialog({
			uuid     :record.get("uuid"),
			listeners:{
				submit:function () {
					this.doReload();
				},
				scope :this
			}
		});
		wnd.show();
	}
});

OMV.NavigationPanelMgr.registerPanel("storage", "greyhole", {
	cls     :OMV.Module.Storage.Greyhole.Admin.SMBPanel,
	position:30,
	title   :"SMB Shares"
});
