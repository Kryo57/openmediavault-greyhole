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
	const xpathRoot = '//services/greyhole';
	public function __construct() {
		$this->methodSchemata = array(
			"setSettings" => array('{
				"type":"object",
				"properties":{
					"enable":{"type":"boolean"},
					"email":{"type":"string"},
                    "loglevel":{"type":"integer","enum":[1,2,3,10]}
				}
			}')
		);
	}

	/**
	 * Safe config getting amd setting
	 */
	public function __call($name, $args) {
		if(substr($name,0,6) == 'config') {
			
			// Correct method name
			$name = substr($name,6);
			$name[0] = strtolower($name[0]);
			
			global $xmlConfig;
			$object = call_user_func_array(array($xmlConfig,$name),$args);
			switch($name) {
				case 'delete':
					if($object === false)
						throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND, $args[0]);
					break;
				case 'save':
					if($object === false)
						throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED, $xmlConfig->getError());
					break;
				case 'set':
				case 'replace':
					if($object === false)
						throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
					break;
				default:
					if(is_null($object))
						throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $args[0]);
			}
			return $object;			
		}
		throw new Exception("Method ".__CLASS__."::".$name." does not exist.");
	}
	
	/**
	 * Verify that the current user is an admin, and validate method args
	 */
	function _validate($mname='',$args=array()) {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
			
		// Check incoming data
		if($mname)
			$this->validateParams($mname, $args);
	}
	
	/**
	 * Get all configuration data for service.
	 * @return array configuration data
	 */
	function getSettings() {
		$this->_validate();
		
		// Get configuration object
		$object = $this->configGet(self::xpathRoot);
		
		// Modify result data
		$object['enable'] = boolval($object['enable']);
		$object['email'] = $object['email'];
		$object['loglevel'] = $object['loglevel'];

		return $object;
	}

	/**
	 * Set configuration data for service.
	 * @param $object array configuration data
	 */
	function setSettings($object) {
		$this->_validate(__METHOD__,func_get_args());
		
		// Prepare configuration data
		$object["enable"] = array_boolval($object, 'enable');
		$object["email"] = $object['email'];
		$object["loglevel"] = $object['loglevel'];
		
		// Set configuration object
		$this->configReplace(self::xpathRoot, $object);

		$this->configSave();
		
		// Notify general configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
			"org.openmediavault.services.greyhole", $object);
	}

}
