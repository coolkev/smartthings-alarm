/**
 *  Arduino Alarm Controller
 *
 *  Copyright 2014 Kevin Lewis
 *
 */
definition(
    name: "Arduino Alarm Controller",
    namespace: "coolkev",
    author: "Kevin Lewis",
    description: "Turn your hardwired alarm into smart sensors",
    category: "Safety & Security",
    iconUrl: "https://s3.amazonaws.com/smartapp-icons/Convenience/Cat-Convenience.png",
    iconX2Url: "https://s3.amazonaws.com/smartapp-icons/Convenience/Cat-Convenience@2x.png")

 
preferences {
	

	section("Zones") {
    	for (int i=1;i<=8;i++) {
			input "zone" + i, title: "Zone " + i, "string", description:"Zone " + i, required: false
        }
        //input "zone1", title: "Zone 1", "string", description:"Zone 1", required: false
        //input "zone2", title: "Zone 2", "string", description:"Zone 2", required: false
	}
    section("Which Arduino shield?") {
		input "arduino", title: "Shield","device.arduinoAlarmDeviceType"
    }    
    
    
}

def installed() {
	log.debug "Installed with settings: ${settings}"
	initialize()
}

def updated() {
	log.debug "Updated with settings: ${settings}"
	unsubscribe()
	initialize()
}


def initialize() {

    
    // Listen to anything which happens on the device
    subscribe(arduino, "zonestatuschanged", zonestatusChanged)
    
    
    settings.each {s ->
    	
    	def name = s.getKey()
		def value = s.getValue()
        
		if (name.startsWith("zone")) {

            log.debug "checking device: ${name}, value: $value"


            def existingDevice = getChildDevice(name)
            if(!existingDevice) {
                log.debug "creating device: ${name}"
            	def childDevice = addChildDevice("coolkev", "Arduino Alarm Virtual Contact Sensor", name, null, [name: "Device.${name}", label: value, completedSetup: true])
            }
            else {
                log.debug "device already exists: ${name}"

            }

        }
    }
    
    
    def delete = getChildDevices().findAll { it.deviceNetworkId.startsWith("zone") && !settings[it.deviceNetworkId] }

    delete.each {
        log.debug "deleting child device: ${it.deviceNetworkId}"
        deleteChildDevice(it.deviceNetworkId)
    }
}

def uninstalled() {
    removeChildDevices(getChildDevices())
}

private removeChildDevices(delete) {
    delete.each {
    	log.debug "deleting child device: ${it.deviceNetworkId}"
        deleteChildDevice(it.deviceNetworkId)
    }
}
def zonestatusChanged(evt)
{
	
    //def value = zigbee.parse(evt)?.text
    
    log.debug "zonestatusChanged ${evt.value}"
    
    
    def parts = evt.value.split();
    
    def zone = parts[0]
    def status = parts[1]
    
    log.debug "zone ${zone}"
    log.debug "status ${status}"
    
    def device = getChildDevice("zone${zone}")
    
    if (device)
    {
    	log.debug "$device statusChanged $status"
    	device.statusChanged(status)
    
    }
}
