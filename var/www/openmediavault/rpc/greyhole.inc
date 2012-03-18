<?php
/**
 * vim: tabstop=4
 *
 * @license		http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author		 Stephane Bocquet <stephane_bocquet@hotmail.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
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

	public function __construct()
	{
		$this->methodSchemata = array(
			"getPoolList"		=> array(
				'{"type":"integer"}', // start
				'{"type":"integer"}', // count
				'{' . $GLOBALS['OMV_JSONSCHEMA_SORTFIELD'] . '}', // sortField
				'{' . $GLOBALS['OMV_JSONSCHEMA_SORTDIR'] . '}' // sortDir
			),
			"getPoolDisk"		=> array(
				'{' . $GLOBALS['OMV_JSONSCHEMA_UUID'] . '}'
			),
			"setPoolDisk"		=> array(
				'{
				"type":"object",
				"properties":{
					"uuid":{' . $GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'] . '},
					"mntentref":{' . $GLOBALS['OMV_JSONSCHEMA_UUID'] . '},
					"min_free":{"type":"integer"}
				}
			}'
			),
			"deletePoolDisk" => array(
				'{' . $GLOBALS['OMV_JSONSCHEMA_UUID'] . '}', // uuid
				'{"type":"boolean"}' // recursive
			),
			"setSettings"		=> array(
				'{
				"type":"object",
				"properties":{
					"enable":{"type":"boolean"},
					"email_to":{"type":"string"},
					"log_level":{"type":"string","enum":["ERROR","WARN","INFO","DEBUG"]},
					"delete_moves_to_trash":{"type":"boolean"},
					"balance_modified_files":{"type":"boolean"},
					"df_cache_time":{"type":"integer"},
					"log_memory_usage":{"type":"boolean"},
					"check_for_open_files":{"type":"boolean"},
					"extraoptions":{"type":"string"},
					"db_host":{"type":"string"},
					"db_user":{"type":"string"},
					"db_pass":{"type":"string"},
					"db_name":{"type":"string"}
				}
			}'
			),
			"getSMBList"		 => array(
				'{"type":"integer"}', // start
				'{"type":"integer"}', // count
				'{' . $GLOBALS['OMV_JSONSCHEMA_SORTFIELD'] . '}', // sortField
				'{' . $GLOBALS['OMV_JSONSCHEMA_SORTDIR'] . '}' // sortDir
			),
			"getSBMShare"		=> array(
				'{' . $GLOBALS['OMV_JSONSCHEMA_UUID'] . '}'
			),
			"setSMBShare"		=> array(
				'{
					"type":"object",
					"properties":{
						"uuid":{' . $GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'] . '},
						"smbref":{' . $GLOBALS['OMV_JSONSCHEMA_UUID'] . '},
						"num_copies":{"type":"integer"},
						"stiky_files":{"type":"boolean"},
						"trash":{"type":"boolean"}
					}
				}'
			),
			"fsck"					 => array(
				'{
					"type":"object",
					"properties":{
						"path":{"type":"string"},
						"email_report":{"type":"boolean"},
						"dont_walk_metadata_store":{"type":"boolean"},
						"find_orphaned_files":{"type":"boolean"},
						"checksums":{"type":"boolean"},
						"delete_orphaned_metadata":{"type":"boolean"}
					}
				}'
			),
		);
	}

	/**
	 * Get all configuration data for service.
	 *
	 * @return array configuration data
	 */
	function getSettings()
	{
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (! $this->hasRole(OMV_ROLE_ADMINISTRATOR))
		{
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath  = "//system/storage/greyhole";
		$object = $xmlConfig->get($xpath);
		if (is_null($object))
		{
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				$xpath);
		}

		// Modify result data
		$object['enable'] = boolval($object['enable']);

		//$object['email_to'] = $object['email_to'];

		//$object['log_level'] = $object['log_level'];

		$object['delete_moves_to_trash']  = boolval($object['delete_moves_to_trash']);
		$object['balance_modified_files'] = boolval($object['balance_modified_files']);

		//$object['df_cache_time']        = $object['df_cache_time'];
		$object['log_memory_usage']     = boolval($object['log_memory_usage']);
		$object['check_for_open_files'] = boolval($object['check_for_open_files']);

		$object['db_host'] = $object['db']['db_host'];
		$object['db_user'] = $object['db']['db_user'];
		$object['db_pass'] = $object['db']['db_pass'];
		$object['db_name'] = $object['db']['db_name'];

		return $object;
	}

	/**
	 * Set configuration data for service.
	 *
	 * @param array $data Settings object
	 *
	 * @throws OMVException
	 */
	function setSettings($data)
	{
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (! $this->hasRole(OMV_ROLE_ADMINISTRATOR))
		{
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get existing configuration object
		$xpath     = "//system/storage/greyhole";
		$oldObject = $xmlConfig->get($xpath);
		if (is_null($oldObject))
		{
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				$xpath);
		}

		if (empty($data['email_to']))
		{
			$xpath_email  = "//system/email";
			$object_email = $xmlConfig->get($xpath_email);
			if (is_null($object_email))
			{
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath_email);
			}
			$data['email_to'] = $object_email['primaryemail'];
		}

		// Prepare configuration data
		$object = array(
			'enable'								 => array_boolval($data, 'enable'),
			'email_to'							 => $data['email_to'],
			'log_level'							=> $data['log_level'],

			'delete_moves_to_trash'	=> array_boolval($data, 'delete_moves_to_trash'),
			'balance_modified_files' => array_boolval($data, 'balance_modified_files'),

			'df_cache_time'					=> $data['df_cache_time'],
			'log_memory_usage'			 => array_boolval($data, 'log_memory_usage'),
			'check_for_open_files'	 => array_boolval($data, 'check_for_open_files'),
			"extraoptions"					 => $data['extraoptions'],

			"pool_disks"						 => isset($oldObject['pool_disks']) ? $oldObject['pool_disks'] : array(),
			"smb_shares"						 => isset($oldObject['smb_shares']) ? $oldObject['smb_shares'] : array(),
			"db"										 => array(
				'db_engine' => 'mysql',
				'db_host'	 => $data['db_host'],
				'db_user'	 => $data['db_user'],
				'db_pass'	 => $data['db_pass'],
				'db_name'	 => $data['db_name'],
			),
		);

		// Set configuration object
		if (FALSE === $xmlConfig->replace($xpath, $object))
		{
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}

		// Save configuration
		if (FALSE === $xmlConfig->save())
		{
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED, $xmlConfig->getError());
		}

		// Notify general configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY, "org.openmediavault.system.storage.greyhole", $data);
	}

	/**
	 * Get Pool List
	 *
	 * @param int		$start
	 * @param int		$count
	 * @param string $sortField
	 * @param string $sortDir
	 *
	 * @return array
	 * @throws OMVException
	 */
	function getPoolList($start, $count, $sortField, $sortDir)
	{
		//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (! $this->hasRole(OMV_ROLE_ADMINISTRATOR))
		{
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration object
		$xpath               = "//system/storage/greyhole/pool_disks/disk";
		$greyhole_pool_disks = $xmlConfig->getList($xpath);
		if (is_null($greyhole_pool_disks))
		{
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				$xpath);
		}

		$cmd = sprintf("sudo greyhole --stats --json");
		OMVUtil::exec($cmd, $output, $result);
		if (isset($output[0]))
		{
			$greyhole_stats = json_decode($output[0]);
		}
		else
		{
			$greyhole_stats = null;
		}
		// Add additional informations.
		foreach ($greyhole_pool_disks as $objectk => &$objectv)
		{
			$objectv['volume'] = gettext("n/a");
			// Get the mount point configuration object to append additional
			// informations, e.g. the devicefile or a modified long
			// description, to the returned objects.
			$xpath        = sprintf("//system/fstab/mntent[uuid='%s']", $objectv['mntentref']);
			$fstab_object = $xmlConfig->get($xpath);
			if (! is_null($fstab_object) && ! empty($fstab_object))
			{
				$fs = new OMVFilesystem($fstab_object['fsname']);
				if (TRUE === $fs->exists())
				{
					$objectv['volume'] = $fs->getDeviceFile();
					$objectv['label']  = $fs->getLabel();
					$objectv['type']   = $fs->getType();
				}
				$objectv['path'] = $fstab_object['dir'] . '/.greyhole';
			}

			if (($greyhole_stats !== null) and isset($greyhole_stats->$objectv['path']))
			{
				$objectv['total_space']               = $greyhole_stats->$objectv['path']->total_space;
				$objectv['used_space']                = $greyhole_stats->$objectv['path']->used_space;
				$objectv['free_space']                = $greyhole_stats->$objectv['path']->free_space;
				$objectv['trash_size']                = $greyhole_stats->$objectv['path']->trash_size;
				$objectv['potential_available_space'] = $greyhole_stats->$objectv['path']->potential_available_space;
			}
		}
		// Filter result
		return $this->applyFilter($greyhole_pool_disks, $start, $count, $sortField, $sortDir);
	}

	/**
	 * Get specific Pool Disk
	 *
	 * @param string $uuid Pool disk UUID
	 *
	 * @return array
	 * @throws OMVException
	 */
	function getPoolDisk($uuid)
	{
		//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (! $this->hasRole(OMV_ROLE_ADMINISTRATOR))
		{
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration object
		$xpath  = "//system/storage/greyhole/pool_disks/disk[uuid='{$uuid}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object))
		{
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				$xpath);
		}
		// Build the full path of the shared folder to display it in the
		// WebGUI.
		$xpath    = sprintf("//system/fstab/mntent[uuid='%s']",
												$object['mntentref']);
		$meobject = $xmlConfig->get($xpath);
		if (is_null($meobject))
		{
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				$xpath);
		}
		return $object;
	}

	/**
	 * Set (add/update) a pool disk object.
	 *
	 * @param array $data Pool disk object
	 *
	 * @throws OMVException
	 */
	function setPoolDisk($data)
	{
		//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (! $this->hasRole(OMV_ROLE_ADMINISTRATOR))
		{
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Prepare configuration data. Use the name of the shared folder
		// as the relative directory name of the share.
		$object = array(
			"uuid"			=> ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMVUtil::uuid() : $data['uuid'],
			"mntentref" => $data['mntentref'],
			"min_free"	=> (int) $data['min_free'],
		);
		// Set configuration data
		$success = FALSE;

		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED'])
		{
			// Check uniqueness
			if (TRUE === $xmlConfig->exists("//system/storage/greyhole/pool_disks/disk[" .
																			"uuid='{$object['uuid']}']")
			)
			{
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS);
			}

			// Append object to configuration
			$success = $xmlConfig->set("//system/storage/greyhole/pool_disks", array("disk" => $object));
		}
		else
		{
			// Get existing configuration object
			$xpath     = "//system/storage/greyhole/pool_disks/disk[uuid='{$data['uuid']}']";
			$oldObject = $xmlConfig->get($xpath);
			if (is_null($oldObject))
			{
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
					$xpath);
			}
			// Update existing configuration object
			$success = $xmlConfig->replace($xpath, $object);
		}

		if (FALSE === $success)
		{
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save())
		{
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
				$xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY, "org.openmediavault.system.storage.greyhole.pool_disks.disk", $object);
	}

	/**
	 * Delete a pool disk
	 *
	 * @param string	$uuid			The UUID of the configuration object to delete.
	 * @param boolean $recursive Remove the shared folder and its content recursively.
	 *
	 * @throws OMVException
	 */
	function deletePoolDisk($uuid, $recursive)
	{
		//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (! $this->hasRole(OMV_ROLE_ADMINISTRATOR))
		{
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());

		// Get the configuration object
		$xpath  = "//system/storage/greyhole/pool_disks/disk[uuid='{$uuid}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object))
		{
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				$xpath);
		}
		// Delete configuration object
		if (FALSE === ($object = $xmlConfig->delete($xpath)))
		{
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
				$xpath);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save())
		{
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
				$xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE, "org.openmediavault.service.greyhole.pool_disks.disk", $object, $recursive);
	}

	/**
	 * Get pool disk candidates
	 *
	 * @return array
	 * @throws OMVException
	 */
	public function getPoolDiskCandidates()
	{
		//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (! $this->hasRole(OMV_ROLE_ADMINISTRATOR))
		{
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get list of mount points, except bind mounts.
		$xpath   = "//system/fstab/mntent[not(contains(opts,'bind'))]";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects))
		{
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				$xpath);
		}
		// Get the filesystem details for each mount point.
		$result = array();
		foreach ($objects as $objectk => $objectv)
		{
			$fs = new OMVFilesystem($objectv['fsname']);
			if (! $fs->exists())
			{
				//				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				//				  sprintf(gettext("Filesystem '%s' not found"),
				//				  $objectv['fsname']));
				continue;
			}
			// Skip the filesystems that are not mounted at the moment.
			if (FALSE === $fs->isMounted())
			{
				continue;
			}

			if (NULL !== ($xmlConfig->get("//system/storage/greyhole/pool_disks/disk[mntentref='{$objectv['uuid']}']")))
			{
				continue;
			}
			// Get the filesystem details.
			$object = array(
				"uuid"				=> $objectv['uuid'],
				"devicefile"	=> $fs->getDeviceFile(),
				"description" => $fs->getDeviceFile() // Default value
			);
			// Get some more filesystem details if possible.
			if (FALSE !== ($fsStats = $fs->getStatistics()))
			{
				$label                 = $fs->getLabel();
				$object['description'] = sprintf(
					gettext("%s (%s available)"), ! empty($label) ?
																							$label : $object['devicefile'],
					binary_format($fsStats['available']));
			}
			$result[] = $object;
		}
		return $result;
	}

	/**
	 * Count pool disks
	 *
	 * @return array
	 * @throws OMVException
	 */
	public function getPoolDiskCount()
	{
		//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (! $this->hasRole(OMV_ROLE_ADMINISTRATOR))
		{
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get list of mount points, except bind mounts.
		$xpath = "//system/storage/greyhole/pool_disks/disk";
		$count = $xmlConfig->count($xpath);
		if (is_null($count))
		{
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				$xpath);
		}
		$result = array();
		for ($i = 1;$i <= $count;$i ++)
		{
			$result[] = array(
				'number' => $i,
				'name'	 => $i,
			);
		}

		return $result;
	}

	/**
	 * Get SMB share candidates
	 *
	 * @return array
	 * @throws OMVException
	 */
	public function getSMBShareCandidates()
	{
		//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (! $this->hasRole(OMV_ROLE_ADMINISTRATOR))
		{
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get list of mount points, except bind mounts.
		$xpath   = "//services/smb/shares/share";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects))
		{
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				$xpath);
		}
		// Get the filesystem details for each mount point.
		$result = array();
		foreach ($objects as $objectk => $objectv)
		{

			if (NULL !== ($xmlConfig->get("//system/storage/greyhole/smb_shares/share[smbref='{$objectv['uuid']}']")))
			{
				continue;
			}
			// Get the filesystem details.
			$object   = array(
				"uuid" => $objectv['uuid'],
				"name" => $objectv['name'],
			);
			$result[] = $object;
		}
		return $result;
	}

	/**
	 * Get SMB list
	 *
	 * @param int		$start
	 * @param int		$count
	 * @param string $sortField
	 * @param string $sortDir
	 *
	 * @return array
	 * @throws OMVException
	 */
	public function getSMBList($start, $count, $sortField, $sortDir)
	{
		//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (! $this->hasRole(OMV_ROLE_ADMINISTRATOR))
		{
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration object
		$xpath   = "//system/storage/greyhole/smb_shares/share";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects))
		{
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				$xpath);
		}
		// Add additional informations.
		foreach ($objects as $objectk => &$objectv)
		{
			$xpath    = sprintf("//services/smb/shares/share[uuid='%s']", $objectv['smbref']);
			$meobject = $xmlConfig->get($xpath);
			if (! is_null($meobject) && ! empty($meobject))
			{
				$objectv['name']    = $meobject['name'];
				$objectv['comment'] = $meobject['comment'];
			}
		}
		// Filter result
		return $this->applyFilter($objects, $start, $count, $sortField,
															$sortDir);
	}


	/**
	 * Get a shared folder config object.
	 *
	 * @param $uuid SMB share UUID
	 *
	 * @return array
	 * @throws OMVException
	 */
	public function getSBMShare($uuid)
	{
		//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();

		if (! $this->hasRole(OMV_ROLE_ADMINISTRATOR))
		{
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration object
		$xpath  = "//system/storage/greyhole/smb_shares/share[uuid='{$uuid}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object))
		{
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				$xpath);
		}
		return $object;
	}

	/**
	 * Set (add/update) a shared folder config object.
	 *
	 * @param array $data
	 *
	 * @throws OMVException
	 */
	public function setSMBShare($data)
	{
		//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (! $this->hasRole(OMV_ROLE_ADMINISTRATOR))
		{
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Prepare configuration data. Use the name of the shared folder
		// as the relative directory name of the share.
		$object = array(
			"uuid"					 => ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMVUtil::uuid() : $data['uuid'],
			"smbref"				 => $data['smbref'],
			"num_copies"		 => (int) $data['num_copies'],
			"stiky_files"		=> array_boolval($data, 'stiky_files'),
			"trash"					=> array_boolval($data, 'trash'),
		);
		// Set configuration data
		$success = FALSE;

		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED'])
		{
			// Check uniqueness
			if (TRUE === $xmlConfig->exists("//system/storage/greyhole/smb_shares/share[" .
																			"uuid='{$object['uuid']}']")
			)
			{
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS);
			}

			// Append object to configuration
			$success = $xmlConfig->set("//system/storage/greyhole/smb_shares", array("share" => $object));
		}
		else
		{
			// Get existing configuration object
			$xpath     = "//system/storage/greyhole/smb_shares/share[uuid='{$data['uuid']}']";
			$oldObject = $xmlConfig->get($xpath);
			if (is_null($oldObject))
			{
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
					$xpath);
			}
			// Update existing configuration object
			$success = $xmlConfig->replace($xpath, $object);
		}

		if (FALSE === $success)
		{
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save())
		{
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
				$xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ? OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY, "org.openmediavault.system.storage.greyhole.smb_shares.share", $object);
	}

	/**
	 * Get fsck candidates
	 *
	 * @return array
	 * @throws OMVException
	 */
	public function getFsckCandidates($start, $count, $sortField, $sortDir, $params)
	{
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (! $this->hasRole(OMV_ROLE_ADMINISTRATOR))
		{
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();

		$type   = $params['type'];
		$result = array();
		// Add additional informations.
		if (in_array($type, array('all', 'smb')))
		{
			// Get SMB Shares
			$xpath               = "//system/storage/greyhole/smb_shares/share";
			$greyhole_smb_shares = $xmlConfig->getList($xpath);
			if (is_null($greyhole_smb_shares))
			{
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);
			}

			foreach ($greyhole_smb_shares as $objectk => &$objectv)
			{
				$xpath      = sprintf("//services/smb/shares/share[uuid='%s']", $objectv['smbref']);
				$smb_object = $xmlConfig->get($xpath);
				if (is_null($smb_object))
				{
					throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);
				}

				$xpath         = sprintf("//system/shares/sharedfolder[uuid='%s']", $smb_object['sharedfolderref']);
				$shared_object = $xmlConfig->get($xpath);
				if (is_null($shared_object))
				{
					throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);
				}

				$xpath        = sprintf("//system/fstab/mntent[uuid='%s']", $shared_object['mntentref']);
				$fstab_object = $xmlConfig->get($xpath);
				if (is_null($fstab_object))
				{
					throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);
				}

				$object   = array(
					'type' => 'SMB Share',
					'name' => $smb_object['name'],
					'show' => 'SMB Share: ' . $smb_object['name'],
					'path' => $fstab_object['dir'] . '/' . $shared_object['reldirpath'],
				);
				$result[] = $object;
			}
		}

		// Get Pool Disks
		if (in_array($type, array('all', 'pool')))
		{
			$xpath               = "//system/storage/greyhole/pool_disks/disk";
			$greyhole_pool_disks = $xmlConfig->getList($xpath);
			if (is_null($greyhole_pool_disks))
			{
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);
			}
			// Add additional informations.
			foreach ($greyhole_pool_disks as $objectk => &$objectv)
			{
				$xpath        = sprintf("//system/fstab/mntent[uuid='%s']", $objectv['mntentref']);
				$fstab_object = $xmlConfig->get($xpath);
				if (is_null($fstab_object))
				{
					throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);
				}

				if (! is_null($fstab_object) && ! empty($fstab_object))
				{
					$fs = new OMVFilesystem($fstab_object['fsname']);
					if (TRUE === $fs->exists())
					{
						$objectv['volume'] = $fs->getDeviceFile();
						$objectv['label']  = $fs->getLabel();
					}
				}

				$object   = array(
					'show' => 'Pool Disk: ' . $objectv['label'] . ' (' . $objectv['volume'] . ')',
					'path' => $fstab_object['dir'] . '/.greyhole',
				);
				$result[] = $object;
			}
		}

		return $result;
	}

	/**
	 * Get pool managment candidates
	 *
	 * @return array
	 * @throws OMVException
	 */
	public function getPoolMngtCandidates()
	{
		//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (! $this->hasRole(OMV_ROLE_ADMINISTRATOR))
		{
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();

		// Get Pool Disks
		$xpath               = "//system/storage/greyhole/pool_disks/disk";
		$greyhole_pool_disks = $xmlConfig->getList($xpath);
		if (is_null($greyhole_pool_disks))
		{
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);
		}
		$result = array();
		// Add additional informations.
		foreach ($greyhole_pool_disks as $objectk => &$objectv)
		{
			$xpath        = sprintf("//system/fstab/mntent[uuid='%s']", $objectv['mntentref']);
			$fstab_object = $xmlConfig->get($xpath);
			if (is_null($fstab_object))
			{
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);
			}

			if (! is_null($fstab_object) && ! empty($fstab_object))
			{
				$fs = new OMVFilesystem($fstab_object['fsname']);
				if (TRUE === $fs->exists())
				{
					$objectv['volume'] = $fs->getDeviceFile();
					$objectv['label']  = $fs->getLabel();
				}
			}

			$object   = array(
				'show' => 'Pool Disk: ' . $objectv['label'] . ' (' . $objectv['volume'] . ')',
				'path' => $fstab_object['dir'] . '/.greyhole',
			);
			$result[] = $object;
		}

		return $result;
	}

	/**
	 * Pool managment
	 *
	 * @param array $data
	 *
	 * @return string
	 * @throws OMVException
	 */
	public function poolMngt($data)
	{
		// Check permissions
		$this->validateSession();
		if (! $this->hasRole(OMV_ROLE_ADMINISTRATOR))
		{
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration data
		$path = $data['path'];

		$greyhole_poolmngt_params = '';

		switch ($data['poolmngt'])
		{
			case 'wait_for':
				$greyhole_poolmngt_params .= '--wait-for=';
				break;
			case 'going':
				$greyhole_poolmngt_params .= '--going=';
				break;
			case 'gone':
				$greyhole_poolmngt_params .= '--gone=';
				break;
			case 'replaced':
				$greyhole_poolmngt_params .= '--replaced=';
				break;
		}
		$cmd = 'sudo greyhole ' . $greyhole_poolmngt_params . $path . ' 2 >&1';
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
		{
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd, implode("\n", $output));
		}

		return 'success';
	}

	/**
	 * Balance
	 *
	 * @return string
	 * @throws OMVException
	 */
	public function balance()
	{
		// Check permissions
		$this->validateSession();
		if (! $this->hasRole(OMV_ROLE_ADMINISTRATOR))
		{
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();

		$cmd = 'sudo greyhole --balance 2 >&1';
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
		{
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd, implode("\n", $output));
		}

		return 'success';
	}

	/**
	 * fsck
	 *
	 * @param array $data
	 *
	 * @return string
	 * @throws OMVException
	 */
	public function fsck($data)
	{
		// Check permissions
		$this->validateSession();
		if (! $this->hasRole(OMV_ROLE_ADMINISTRATOR))
		{
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration data

		//throw new Exception(print_r($data, true));

		array_boolval($data, 'delete_moves_to_trash');

		$path = $data['path'];

		$email_report         = array_boolval($data, 'email_report');
		$greyhole_fsck_params = '';

		if ($email_report === true)
		{
			$greyhole_fsck_params .= ' --email-report';
		}

		$dont_walk_metadata_store = array_boolval($data, 'dont_walk_metadata_store');
		if ($dont_walk_metadata_store === true)
		{
			$greyhole_fsck_params .= ' --dont-walk-metadata-store';
		}

		$find_orphaned_files = array_boolval($data, 'find_orphaned_files');
		if ($find_orphaned_files === true)
		{
			$greyhole_fsck_params .= ' --find-orphaned-files';
		}

		$checksums = array_boolval($data, 'checksums');
		if ($checksums === true)
		{
			$greyhole_fsck_params .= ' --checksums';
		}

		$delete_orphaned_metadata = array_boolval($data, 'delete_orphaned_metadata');
		if ($delete_orphaned_metadata === true)
		{
			$greyhole_fsck_params .= ' --delete-orphaned-metadata';
		}

		$cmd = 'sudo greyhole --fsck ' . $greyhole_fsck_params . ' --dir=' . $path . ' 2 >&1';
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
		{
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd, implode("\n", $output));
		}

		return 'success';
	}

	/**
	 * Cancel fsck
	 *
	 * @return string
	 * @throws OMVException
	 */
	public function unfsck()
	{
		// Check permissions
		$this->validateSession();
		if (! $this->hasRole(OMV_ROLE_ADMINISTRATOR))
		{
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();

		$cmd = 'sudo greyhole --cancel-fsck 2 >&1';
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
		{
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd, implode("\n", $output));
		}

		return 'success';
	}

	/**
	 * Empty trash
	 *
	 * @return string
	 * @throws OMVException
	 */
	public function emptyTrash()
	{
		// Check permissions
		$this->validateSession();
		if (! $this->hasRole(OMV_ROLE_ADMINISTRATOR))
		{
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();

		$cmd = 'sudo greyhole --empty-trash 2 >&1';
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
		{
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd, implode("\n", $output));
		}

		return 'success';
	}

}
