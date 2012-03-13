/**
 * Created by JetBrains PhpStorm.
 * User: mbeck
 * Date: 11.12.11
 * Time: 20:11
 * To change this template use File | Settings | File Templates.
 */

// require("js/omv/module/greyhole/admin.js")
// require("js/omv/util/Format.js")

OMV.Module.Diagnostics.LogPlugin.Greyhole = function (config) {
	var initialConfig = {
		title    :"Greyhole",
		stateId  :"a4150311-5e3a-4693-8381-933088a9f98f",
		columns  :[
			{
				header   :"Date & Time",
				sortable :true,
				dataIndex:"date",
				id       :"date",
				width    :50,
				renderer :OMV.util.Format.localeTimeRenderer()
			},
			{
				header   :"Component",
				sortable :true,
				dataIndex:"component",
				id       :"component",
				width    :20
			},
			{
				header   :"Event",
				sortable :true,
				dataIndex:"event",
				id       :"event"
			}
		],
		rpcArgs  :"greyhole",
		rpcFields:[
			{ name:"date" },
			{ name:"component" },
			{ name:"event" }
		]
	};
	Ext.apply(initialConfig, config);
	OMV.Module.Diagnostics.LogPlugin.Greyhole.superclass.constructor.call(
					this, initialConfig);
};

Ext.extend(OMV.Module.Diagnostics.LogPlugin.Greyhole,
				OMV.Module.Diagnostics.LogPlugin, {
				});
OMV.preg("log", "greyhole", OMV.Module.Diagnostics.LogPlugin.Greyhole);