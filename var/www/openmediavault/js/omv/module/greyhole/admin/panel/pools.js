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

// require("js/omv/module/greyhole/panel/navigation.js")

// require("js/omv/module/greyhole/admin/dialog/pooldisk.js")

Ext.ns("OMV.Module.Storage.Greyhole.Admin");

/**
 * @class OMV.Module.Storage.Greyhole.Admin.PoolsPanel
 * @derived OMV.grid.TBarGridPanel
 * Display list of configured filesystems.
 */
OMV.Module.Storage.Greyhole.Admin.PoolsPanel = function(config) {
	var initialConfig = {
		hidePagingToolbar: false,
		autoReload: true,
		stateId: "85f1cbf2-23d3-4960-a803-b7fc34d42235",
		colModel: new Ext.grid.ColumnModel({
			columns: [{
				header: "Volume",
				sortable: true,
				dataIndex: "volume",
				id: "volume"
			},{
				header: "Label",
				sortable: true,
				dataIndex: "label",
				id: "label"
			},{
				header: "FS Type",
				sortable: true,
				dataIndex: "type",
				id: "type"
			},{
				header: "Min Free",
				sortable: true,
				dataIndex: "min_free",
				id: "type"
			},{
				header: "Status",
				sortable: true,
				dataIndex: "status",
				id: "status",
				renderer: this.statusRenderer,
				scope: this
			}]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.Greyhole.Admin.PoolsPanel.superclass.constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.Greyhole.Admin.PoolsPanel, OMV.grid.TBarGridPanel, {
	initComponent : function() {
		this.store = new OMV.data.Store({
			autoLoad: true,
			remoteSort: false,
			proxy: new OMV.data.DataProxy("Greyhole", "getPoolList"),
			reader: new Ext.data.JsonReader({
				idProperty: "uuid",
				totalProperty: "total",
				root: "data",
				fields: [
					{ name: "uuid" },
					{ name: "volume" },
					{ name: "label" },
					{ name: "type" },
					{ name: "min_free" },
					{ name: "type" }
    			]
			})
		});
		OMV.Module.Storage.Greyhole.Admin.PoolsPanel.superclass.initComponent.apply(this, arguments);
		// Register event handler
		// Reselect previous selected rows after the store has been
		// reloaded, e.g. to make sure toolbar is updated depending on
		// the latest row record values.
		this.getSelectionModel().previousSelections = [];
		this.store.on("beforeload", function(store, options) {
			  var sm = this.getSelectionModel();
			  var records = sm.getSelections();
			  sm.previousSelections = [];
			  Ext.each(records, function(record, index) {
				  sm.previousSelections.push(record.get("uuid"));
			  }, this);
		  }, this);
		this.store.on("load", function(store, records, options) {
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

	initToolbar : function() {
		var tbar = OMV.Module.Storage.Greyhole.Admin.PoolsPanel.superclass.initToolbar.apply(this);
		return tbar;
	},

	cbAddBtnHdl : function() {
		var wnd = new OMV.Module.Storage.Greyhole.Admin.PoolDiskPanel({
			uuid: OMV.UUID_UNDEFINED,
			listeners: {
				submit: function() {
					this.doReload();
				},
				scope: this
			}
		});
		wnd.show();
	},

	cbEditBtnHdl : function() {
		var selModel = this.getSelectionModel();
		var record = selModel.getSelected();
		var wnd = new OMV.Module.Storage.Greyhole.Admin.PoolDiskPanel({
			uuid: record.get("uuid"),
			listeners: {
				submit: function() {
					this.doReload();
				},
				scope: this
			}
		});
		wnd.show();
	},

	startDeletion: function(model, records) {
		if(records.length <= 0)
			return;
		OMV.MessageBox.show({
			title: "Delete Pool Disk",
			msg: "Do you want to remove the content of the pool disk directory " +
				"recursively? Note, the data will be permanently " +
				"deleted then. Select 'No' to delete the pool disk directory only " +
				"or 'Cancel' to abort.",
			buttons: Ext.Msg.YESNOCANCEL,
			fn: function(answer) {
				this.deleteRecursive = false;
				switch(answer) {
				case "yes":
					OMV.MessageBox.show({
						title: "Confirmation",
						msg: "Do you really want to remove the pool disk " +
							"directory content?",
						buttons: OMV.Msg.YESCANCEL,
						fn: function(answer) {
							if(answer === "yes") {
								this.deleteRecursive = true;
								OMV.Module.Storage.Greyhole.Admin.PoolsPanel.superclass.startDeletion.call(this, model, records);
							}
						},
						scope: this,
						icon: Ext.Msg.QUESTION
					});
					break;
				case "no":
					OMV.Module.Storage.Greyhole.Admin.PoolsPanel.superclass.startDeletion.call(this, model, records);
					break;
				case "cancel":
					break;
				}
			},
			scope: this,
			icon: Ext.Msg.QUESTION
		});
	},

	doDeletion : function(record) {
		OMV.Ajax.request(this.cbDeletionHdl, this, "Greyhole", "deletePoolDisk", [ record.get("uuid"), this.deleteRecursive ]);
	},

	afterDeletion : function() {
		OMV.Module.Storage.Greyhole.Admin.PoolsPanel.superclass.afterDeletion.apply(this, arguments);
		delete this.deleteRecursive;
	},

	statusRenderer : function(val, cell, record, row, col, store) {
		switch (val) {
		case 1:
			val = "Online";
			break;
		case 2:
			val = "<img border='0' src='images/wait.gif'> Initializing";
			break;
		default:
			val = "Missing";
			break;
		}
		return val;
	}
});
OMV.NavigationPanelMgr.registerPanel("storage", "greyhole", {
	cls: OMV.Module.Storage.Greyhole.Admin.PoolsPanel,
	position : 20,
	title : "Pools"
});
