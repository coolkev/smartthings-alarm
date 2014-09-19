/**
 *  ArduinoAlarmDeviceType
 *
 *  Copyright 2014 Kevin Lewis
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 *  in compliance with the License. You may obtain a copy of the License at:
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software distributed under the License is distributed
 *  on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License
 *  for the specific language governing permissions and limitations under the License.
 *
 */
metadata {
	definition (name: "ArduinoAlarmDeviceType", namespace: "coolkev", author: "Kevin Lewis") {
    
    	fingerprint profileId: "0104", deviceId: "0138", inClusters: "0000"
	}
	tiles {
        standardTile("shield", "device.shield", width: 2, height: 2) {
                state "default", icon:"st.shields.shields.arduino", backgroundColor:"#ffffff"
        }
        main "shield"
		details "shield"
    }    
    simulator {
		// status messages
		status "open":   "zonestatus 1 open"
		status "closed": "zonestatus 1 closed"
	}

}

Map parse(String description) {
log.debug "parent ${parent}"
log.debug "Parse called ${description}"
	def name = null
	def value = zigbee.parse(description)?.text
    
    value = value==null ? description : value
    
    log.debug "value: ${value}"
	if (value.startsWith("zonestatus")) {
		name = "zonestatuschanged"
		
        def parts = value.split();
        
        value = parts[1] + ' ' + parts[2]
        
        
	}
	
	def result = createEvent(name: name, value: value)
	log.debug "Parse returned ${result?.descriptionText}"
	return result
}
