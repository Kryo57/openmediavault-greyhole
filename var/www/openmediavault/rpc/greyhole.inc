<?php
/**
 * vim: tabstop=4
 *
 * @license		http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author		Stephane Bocquet <stephane_bocquet@hotmail.com>
 * @copyright	Copyright (c) 2011 Ian Moore
 * @version		$Id: greyhole.inc 12 2011-11-07 18:52:10Z stephane_bocquet@hotmail.com $
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file. If not, see <http://www.gnu.org/licenses/>.
 *
 */

require_once("openmediavault/object.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class GreyholeRpc extends OMVRpc {

	public function __construct() {
		$this->methodSchemata = array(
			"getPoolList" => array(
				'{"type":"integer"}', // start
				'{"type":"integer"}', // count
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'}', // sortField
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}' // sortDir
			),
			"getPoolDisk" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"setPoolDisk" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"mntentref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"deletePoolDisk" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}', // uuid
				'{"type":"boolean"}' // recursive
			),
			"setSettings" => array('{
				"type":"object",
				"properties":{
					"enable":{"type":"boolean"},
					"email_to":{"type":"string"},
          "log_level":{"type":"string","enum":["ERROR","WARN","INFO","DEBUG"]},
          "delete_moves_to_trash":{"type":"boolean"},
          "balance_modified_files":{"type":"boolean"},
          "df_cache_time":{"type":"integer"},
          "log_memory_usage":{"type":"boolean"},
          "check_for_open_files":{"type":"boolean"}
				}
			}')
		);
	}

	/**
	 * Get all configuration data for service.
	 * @return array configuration data
	 */
	function getSettings() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//services/greyhole";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				$xpath);
		}

		// Modify result data
		$object['enable'] = boolval($object['enable']);
		$object['email_to'] = $object['email_to'];
		$object['log_level'] = $object['log_level'];

		$object['delete_moves_to_trash'] = boolval($object['delete_moves_to_trash']);
		$object['balance_modified_files'] = boolval($object['balance_modified_files']);

		$object['df_cache_time'] = $object['df_cache_time'];
		$object['log_memory_usage'] = boolval($object['log_memory_usage']);
		$object['check_for_open_files'] = boolval($object['check_for_open_files']);

		return $object;
	}

	/**
	 * Set configuration data for service.
	 * @param $object array configuration data
	 */
	function setSettings($object) {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get existing configuration object
		$xpath = "//services/greyhole";
		$oldObject = $xmlConfig->get($xpath);
		if (is_null($oldObject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				$xpath);
		}

		// Prepare configuration data
		$object = array(
			'enable' => array_boolval($object, 'enable'),
			'email_to' => $object['email_to'],
			'log_level' => $object['log_level'],

			'delete_moves_to_trash' => array_boolval($object, 'delete_moves_to_trash'),
			'balance_modified_files' => array_boolval($object, 'balance_modified_files'),

			'df_cache_time' => $object['df_cache_time'],
			'log_memory_usage' => array_boolval($object, 'log_memory_usage'),
			'check_for_open_files' => array_boolval($object, 'check_for_open_files'),

			"pool_disks" => isset($oldObject['pool_disks']) ? $oldObject['pool_disks'] : array(),
		);

		// Set configuration object
		if (FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}

		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED, $xmlConfig->getError());
		}

		// Notify general configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY, "org.openmediavault.services.greyhole", $object);
	}

	/**
	 * Get list of shared folder configuration objects.
	 */
	function getPoolList($start, $count, $sortField, $sortDir) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration object
		$xpath = "//services/greyhole/pool_disks";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				$xpath);
		}
		// Add additional informations.
		foreach ($objects as $objectk => &$objectv) {
			$objectv['volume'] = gettext("n/a");
			// Get the mount point configuration object to append additional
			// informations, e.g. the devicefile or a modified long
			// description, to the returned objects.
			$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
				$objectv['mntentref']);
			$meobject = $xmlConfig->get($xpath);
			if (!is_null($meobject) && !empty($meobject)) {
				$fs = new OMVFilesystem($meobject['fsname']);
				if (TRUE === $fs->exists()) {
					$objectv['volume'] = $fs->getDeviceFile();
					$objectv['longdescription'] = sprintf(
						gettext("%s [on %s]"), $objectv['longdescription'],
						$fs->getDeviceFile());
				}
			}
		}
		// Filter result
		return $this->applyFilter($objects, $start, $count, $sortField,
			$sortDir);
	}

	public function getPoolDiskCandidates() {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get list of mount points, except bind mounts.
		$xpath = "//system/fstab/mntent[not(contains(opts,'bind'))]";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				$xpath);
		}
		// Get the filesystem details for each mount point.
		$result = array();
		foreach ($objects as $objectk => $objectv) {
			$fs = new OMVFilesystem($objectv['fsname']);
			if (!$fs->exists()) {
//				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
//				  sprintf(gettext("Filesystem '%s' not found"),
//				  $objectv['fsname']));
				continue;
			}
			// Skip the filesystems that are not mounted at the moment.
			if (FALSE === $fs->isMounted())
				continue;
			// Get the filesystem details.
			$object = array(
				"uuid" => $objectv['uuid'],
				"devicefile" => $fs->getDeviceFile(),
				"description" => $fs->getDeviceFile() // Default value
			);
			// Get some more filesystem details if possible.
			if (FALSE !== ($fsStats = $fs->getStatistics())) {
				$label = $fs->getLabel();
				$object['description'] = sprintf(
					gettext("%s (%s available)"), !empty($label) ?
					$label : $object['devicefile'],
					binary_format($fsStats['available']));
			}
			$result[] = $object;
		}
		return $result;
	}

	/**
		 * Get a shared folder config object.
		 */
		function getPoolDisk($uuid) {
	//		$this->debug(var_export(func_get_args(), TRUE));
			global $xmlConfig;
			// Check permissions
			$this->validateSession();
			if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
				throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
			}
			$this->commitSession();
			// Validate method parameters
			$this->validateParams(__METHOD__, func_get_args());
			// Get configuration object
			$xpath = "//service/greyhole/pool_disks/disk[uuid='{$uuid}']";
			$object = $xmlConfig->get($xpath);
			if (is_null($object)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			// Build the full path of the shared folder to display it in the
			// WebGUI.
			$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
			  $object['mntentref']);
			$meobject = $xmlConfig->get($xpath);
			if (is_null($meobject)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			return $object;
		}

		/**
		 * Set (add/update) a shared folder config object.
		 */
		function setPoolDisk($data) {
	//		$this->debug(var_export(func_get_args(), TRUE));
			global $xmlConfig;
			// Check permissions
			$this->validateSession();
			if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
				throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
			}
			$this->commitSession();
			// Validate method parameters
			$this->validateParams(__METHOD__, func_get_args());
			// Prepare configuration data. Use the name of the shared folder
			// as the relative directory name of the share.
			$object = array(
				"uuid" => ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMVUtil::uuid() : $data['uuid'],
				"mntentref" => $data['mntentref'],
			);
			// Set configuration data
			$success = FALSE;

			if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
				// Check uniqueness
				if (TRUE === $xmlConfig->exists("//service/greyhole/pool_disks/disk[" .
					"uuid='{$object['uuid']}']")) {
					throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS);
				}

				// Append object to configuration
				$success = $xmlConfig->set("//service/greyhole/pool_disks", array("disk" => $object));
			} else {
				// Get existing configuration object
				$xpath = "//service/greyhole/pool_disks/disk[uuid='{$data['uuid']}']";
				$oldObject = $xmlConfig->get($xpath);
				if (is_null($oldObject)) {
					throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
						$xpath);
				}
				// Update existing configuration object
				$success = $xmlConfig->replace($xpath, $object);
			}

			if (FALSE === $success) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
			}
			// Save configuration
			if (FALSE === $xmlConfig->save()) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
				  $xmlConfig->getError());
			}
			// Notify configuration changes
			$dispatcher = &OMVNotifyDispatcher::getInstance();
			$dispatcher->notify(($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
			  "org.openmediavault.service.greyhole.pool_disks.disk", $object);
		}

		/**
		 * Delete a shared folder config object.
		 * @param uuid The UUID of the configuration object to delete.
		 * @param recursive Remove the shared folder and its content recursively.
		 */
		function deletePoolDisk($uuid, $recursive) {
	//		$this->debug(var_export(func_get_args(), TRUE));
			global $xmlConfig;
			// Check permissions
			$this->validateSession();
			if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
				throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
			}
			$this->commitSession();
			// Validate method parameters
			$this->validateParams(__METHOD__, func_get_args());

			// Get the configuration object
			$xpath = "//service/greyhole/pool_disks/disk[uuid='{$uuid}']";
			$object = $xmlConfig->get($xpath);
			if (is_null($object)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			// Notify configuration changes
			$dispatcher = &OMVNotifyDispatcher::getInstance();
			$dispatcher->notify(OMV_NOTIFY_PREDELETE,
			  "org.openmediavault.service.greyhole.pool_disks.disk", $object,
			  $recursive);
			// Delete configuration object
			if (FALSE === ($object = $xmlConfig->delete($xpath))) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
				  $xpath);
			}
			// Save configuration
			if (FALSE === $xmlConfig->save()) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
				  $xmlConfig->getError());
			}
			// Notify configuration changes
			$dispatcher->notify(OMV_NOTIFY_DELETE,
			  "org.openmediavault.service.greyhole.pool_disks.disk", $object,
			  $recursive);
		}

}
