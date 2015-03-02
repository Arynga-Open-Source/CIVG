/* Manifest used to define access privilegies for cloudeebus. */
var manifest = {
    name: "cloudeebus",
    key: "secret",
    permissions: [
        "net.dumm"
    ]
};

/* Variables purposed for cloudeebus. */
var bus;
var dumm;

var dummBusName = 'net.dumm';
var dummPath = '/';
var dummIf = 'net.dumm.download';

function makeDBusObj(fields) {
    var names = fields.split(' ');
    var count = names.length;
    function constructor() {
        for (var i = 0; i < count; i++) {
            this[names[i]] = arguments[i];
        }
    }
    return constructor;
}

var dBusObj = makeDBusObj("busName objectPath introspectCB errorCB");
var dummObj = new dBusObj(dummBusName, dummPath, dummIntrospected, errorCB);

/* Method to establish connection to CarSync objects on DBUS */
function connected(){
    cloudeebus.log("In method Connected()");
    cloudeebus.log("Connect to Websocket on port 9000\n");

    /* Use proper DBus bus - by default we are using session */
    bus = cloudeebus.SessionBus();
    if(!bus)
    {
    	cloudeebus.log("Error: Bus is empty\n");
    }
    else
    {
    	dumm = bus.getObject(dummObj.busName, dummObj.objectPath, dummObj.introspectCB, dummObj.errorCB);
    	if(dumm)
    	{
    		cloudeebus.log("Method Connected() done\n");
    	}    	
    }
}

function errorCB(error) {
    cloudeebus.log("error: " + error + "\n");
}

function dummIntrospected(obj){
    cloudeebus.log("DUMM working");
}

