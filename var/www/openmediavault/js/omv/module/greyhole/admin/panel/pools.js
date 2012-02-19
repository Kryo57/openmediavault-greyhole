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

// require("js/omv/module/greyhole/admin/dialog/pooldisk.js")
// require("js/omv/module/greyhole/admin/dialog/poolmngt.js")
// require("js/omv/module/greyhole/admin/dialog/fsck.js")

// require("js/omv/module/greyhole/util/Format.js")

Ext.ns("OMV.Module.Storage.Greyhole.Admin");

/**
 * @class OMV.Module.Storage.Greyhole.Admin.PoolsPanel
 * @derived OMV.grid.TBarGridPanel
 * Display list of configured filesystems.
 */
OMV.Module.Storage.Greyhole.Admin.PoolsPanel = function (config) {
	var initialConfig = {
		hidePagingToolbar:false,
		autoReload       :true,
		stateId          :"85f1cbf2-23d3-4960-a803-b7fc34d42235",
		colModel         :new Ext.grid.ColumnModel({
			columns:[
				{
					header   :"Volume",
					sortable :true,
					dataIndex:"volume",
					id       :"volume",
					width    :50
				},
				{
					header   :"Label",
					sortable :true,
					dataIndex:"label",
					id       :"label"
				},
				{
					header   :"Filesystem",
					sortable :true,
					dataIndex:"type",
					id       :"type",
					width    :50
				},
				{
					header   :"Path",
					sortable :true,
					dataIndex:"path",
					id       :"path",
					width    :200
				},
				{
					header   :"Space",
					sortable :true,
					dataIndex:"percent_space",
					id       :"percent_space",
					renderer :this.space_renderer,
					scope    :this
				},
				{
					header   :"Trash",
					sortable :true,
					dataIndex:"trash_size",
					id       :"trash_size",
					renderer :this.trash_renderer,
					scope    :this,
					width    :50
				},
				{
					header   :"Min Free",
					sortable :true,
					dataIndex:"min_free",
					id       :"min_free",
					renderer :this.min_free_renderer,
					scope    :this,
					width    :50
				}
			]
		})
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Storage.Greyhole.Admin.PoolsPanel.superclass.constructor.call(this, initialConfig);
};
Ext.extend(OMV.Module.Storage.Greyhole.Admin.PoolsPanel, OMV.grid.TBarGridPanel, {
	initComponent:function () {
		this.store = new OMV.data.Store({
			autoLoad  :true,
			remoteSort:false,
			proxy     :new OMV.data.DataProxy("Greyhole", "getPoolList"),
			reader    :new Ext.data.JsonReader({
				idProperty   :"uuid",
				totalProperty:"total",
				root         :"data",
				fields       :[
					{ name:"uuid" },
					{ name:"volume" },
					{ name:"label" },
					{ name:"type" },
					{ name:"path" },
					{ name:"total_space" },
					{ name:"used_space" },
					{ name:"free_space" },
					{ name:"trash_size" },
					{ name:"potential_available_space" },
					{ name:"min_free" }
				]
			})
		});
		OMV.Module.Storage.Greyhole.Admin.PoolsPanel.superclass.initComponent.apply(this, arguments);
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
		var tbar = OMV.Module.Storage.Greyhole.Admin.PoolsPanel.superclass.initToolbar.apply(this);

		tbar.insert(3, {
			id     :this.getId() + "-poolmngt",
			xtype  :"button",
			text   :"Pool management",
			icon   :"images/greyhole-poolmngt.png",
			handler:this.cbpoolmngtBtnHdl,
			scope  :this
		});

		tbar.insert(4, {
			id     :this.getId() + "-balance",
			xtype  :"button",
			text   :"Files balance",
			icon   :"images/greyhole-balance.png",
			handler:this.cbbalanceBtnHdl,
			scope  :this
		});

		tbar.insert(5, {
			id     :this.getId() + "-fsck",
			xtype  :"button",
			text   :"Files check",
			icon   :"images/greyhole-fsck.png",
			handler:this.cbfsckBtnHdl,
			scope  :this
		});

		tbar.insert(6, {
			id     :this.getId() + "-unfsck",
			xtype  :"button",
			text   :"Cancel all checks",
			icon   :"images/greyhole-unfsck.png",
			handler:this.cbunfsckBtnHdl,
			scope  :this
		});

		tbar.insert(7, {
			id     :this.getId() + "-emptytrash",
			xtype  :"button",
			text   :"Empty trash",
			icon   :"images/greyhole-emptytrash.png",
			handler:this.cbemptytrashBtnHdl,
			scope  :this
		});

		return tbar;
	},

	cbAddBtnHdl:function () {
		var wnd = new OMV.Module.Storage.Greyhole.Admin.PoolDiskDialog({
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
		var wnd = new OMV.Module.Storage.Greyhole.Admin.PoolDiskDialog({
			uuid     :record.get("uuid"),
			listeners:{
				submit:function () {
					this.doReload();
				},
				scope :this
			}
		});
		wnd.show();
	},

	/** POOL MANAGEMENT HANDLER */
	cbpoolmngtBtnHdl:function () {
		var wnd = new OMV.Module.Storage.Greyhole.Admin.PoolMngtDialog({
			listeners:{
				success:function (wnd, path, diskmngt) {
					this.dopoolmngt(path, diskmngt);
				},
				scope  :this
			}
		});
		wnd.show();
	},
	dopoolmngt      :function (path, diskmngt) {
		OMV.Ajax.request(this.cbpoolmngtLHdl, this, "Greyhole", "poolmngt", [
			{
				path    :String(path),
				diskmngt:String(diskmngt)
			}
		]);
	},
	cbpoolmngtLHdl  :function (id, response, error) {
		if (error !== null) {
			// Display error message
			OMV.MessageBox.error(null, error);
		} else {
			OMV.MessageBox.hide();
			this.doReload();
		}
	},
	/** /POOL MANAGEMENT HANDLER */

	/** BALANCE HANDLER */
	cbbalanceBtnHdl:function () {
		this.dobalance();
	},
	dobalance      :function () {
		OMV.Ajax.request(this.cbbalanceLHdl, this, "Greyhole", "balance", []);
	},
	cbbalanceLHdl  :function (id, response, error) {
		if (error !== null) {
			// Display error message
			OMV.MessageBox.error(null, error);
		} else {
			OMV.MessageBox.hide();
			this.doReload();
		}
	},
	/** /BALANCE HANDLER */

	/** FSCK HANDLER */
	cbfsckBtnHdl:function () {
		var wnd = new OMV.Module.Storage.Greyhole.Admin.FSCKDialog({
			listeners:{
				success:function (wnd, path, email_report, dont_walk_metadata_store, find_orphaned_files, checksums, delete_rphaned_metadata) {
					this.dofsck(path, email_report, dont_walk_metadata_store, find_orphaned_files, checksums, delete_rphaned_metadata);
				},
				scope  :this
			}
		});
		wnd.show();
	},
	dofsck      :function (path, email_report, dont_walk_metadata_store, find_orphaned_files, checksums, delete_rphaned_metadata) {
		OMV.Ajax.request(this.cbfsckLHdl, this, "Greyhole", "fsck", [
			{
				path                    :String(path),
				email_report            :Boolean(email_report),
				checksums               :Boolean(checksums),
				dont_walk_metadata_store:Boolean(dont_walk_metadata_store),
				find_orphaned_files     :Boolean(find_orphaned_files),
				delete_orphaned_metadata:Boolean(delete_rphaned_metadata)
			}
		]);
	},
	cbfsckLHdl  :function (id, response, error) {
		if (error !== null) {
			// Display error message
			OMV.MessageBox.error(null, error);
		} else {
			OMV.MessageBox.hide();
			this.doReload();
		}
	},
	/** /FSCK HANDLER */

	/** CANCEL FSCK HANDLER */
	cbunfsckBtnHdl:function () {
		this.dounfsck();
	},
	dounfsck      :function () {
		OMV.Ajax.request(this.cbunfsckLHdl, this, "Greyhole", "unfsck", []);
	},
	cbunfsckLHdl  :function (id, response, error) {
		if (error !== null) {
			// Display error message
			OMV.MessageBox.error(null, error);
		} else {
			OMV.MessageBox.hide();
			this.doReload();
		}
	},
	/** /CANCEL FSCK HANDLER */

	/** EMPTY TRASH HANDLER */
	cbemptytrashBtnHdl:function () {
		this.doemptytrash();
	},
	doemptytrash      :function () {
		OMV.Ajax.request(this.cbemptytrashLHdl, this, "Greyhole", "emptytrash", []);
	},
	cbemptytrashLHdl  :function (id, response, error) {
		if (error !== null) {
			// Display error message
			OMV.MessageBox.error(null, error);
		} else {
			OMV.MessageBox.hide();
			this.doReload();
		}
	},
	/** /EMPTY TRASH HANDLER */

	startDeletion     :function (model, records) {
		if (records.length <= 0)
			return;
		OMV.MessageBox.show({
			title  :"Delete Pool Disk",
			msg    :"Do you want to remove the content of the pool disk directory " +
							"recursively? Note, the data will be permanently " +
							"deleted then. Select 'No' to delete the pool disk directory only " +
							"or 'Cancel' to abort.",
			buttons:Ext.Msg.YESNOCANCEL,
			fn     :function (answer) {
				this.deleteRecursive = false;
				switch (answer) {
					case "yes":
						OMV.MessageBox.show({
							title  :"Confirmation",
							msg    :"Do you really want to remove the pool disk " +
											"directory content?",
							buttons:OMV.Msg.YESCANCEL,
							fn     :function (answer) {
								if (answer === "yes") {
									this.deleteRecursive = true;
									OMV.Module.Storage.Greyhole.Admin.PoolsPanel.superclass.startDeletion.call(this, model, records);
								}
							},
							scope  :this,
							icon   :Ext.Msg.QUESTION
						});
						break;
					case "no":
						OMV.Module.Storage.Greyhole.Admin.PoolsPanel.superclass.startDeletion.call(this, model, records);
						break;
					case "cancel":
						break;
				}
			},
			scope  :this,
			icon   :Ext.Msg.QUESTION
		});
	},

	doDeletion:function (record) {
		OMV.Ajax.request(this.cbDeletionHdl, this, "Greyhole", "deletePoolDisk", [ record.get("uuid"), this.deleteRecursive ]);
	},

	afterDeletion:function () {
		OMV.Module.Storage.Greyhole.Admin.PoolsPanel.superclass.afterDeletion.apply(this, arguments);
		delete this.deleteRecursive;
	},

	space_renderer:function (val, cell, record, row, col, store) {
		var total_space = parseInt(record.get("total_space")) * 1024;
		var used_space = parseInt(record.get("used_space")) * 1024;
		var free_space = parseInt(record.get("free_space")) * 1024;
		var percentage = parseFloat(used_space / total_space);

		if (-1 == total_space) {
			return val;
		}
		var id = Ext.id();
		(function () {
			new Ext.ProgressBar({
				renderTo:id,
				value   :percentage,
				text    :OMV.Module.Storage.Greyhole.Util.Format.bytesToSize(used_space) + '/' + OMV.Module.Storage.Greyhole.Util.Format.bytesToSize(total_space) + ' (' + parseInt(percentage * 100) + '%)'
			});
		}).defer(25);
		return '<div id="' + id + '"></div>';
	},

	min_free_renderer:function (val, cell, record, row, col, store) {
		val = val + ' GiB';
		return val;
	},

	trash_renderer:function (val, cell, record, row, col, store) {
		var trash = parseInt(val) * 1024;
		return OMV.Module.Storage.Greyhole.Util.Format.bytesToSize(trash);
	}
});

OMV.NavigationPanelMgr.registerPanel("storage", "greyhole", {
	cls     :OMV.Module.Storage.Greyhole.Admin.PoolsPanel,
	position:20,
	title   :"Pool"
});
