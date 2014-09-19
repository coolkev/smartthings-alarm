/**
 *  Arduino Alarm Virtual Contact Sensor
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
	definition (name: "Arduino Alarm Virtual Contact Sensor", namespace: "coolkev", author: "Kevin Lewis") {
		capability "Contact Sensor"
        command "statusChanged"
	}
    
	// simulator metadata
	simulator {
		// status messages
		status "open":   "zone report :: type: 19 value: 0031"
		status "closed": "zone report :: type: 19 value: 0030"
	}

	// UI tile definitions
	tiles {
		standardTile("contact", "device.contact", width: 2, height: 2,canChangeIcon: true) {
			state "open", label: '${name}', icon: "st.contact.contact.open", backgroundColor: "#ffa81e"
			state "closed", label: '${name}', icon: "st.contact.contact.closed", backgroundColor: "#79b821"
		}

		main "contact"
		details "contact"
	}
}

// handle commands
def statusChanged(status) {
	log.debug "statusChanged"
    sendEvent (name: "contact", value: status, isStateChange:true)
}
