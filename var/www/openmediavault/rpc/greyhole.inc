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
			"setSettings" => array('{
				"type":"object",
				"properties":{
					"enable":{"type":"boolean"},
					"email_to":{"type":"string"},
          "log_level":{"type":"integer","enum":[1,2,3,10]}
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

}
