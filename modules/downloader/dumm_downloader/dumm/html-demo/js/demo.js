/* Manifest used to define access privilegies for cloudeebus. */
var manifest = {
    name: "cloudeebus",
    key: "secret",
    permissions: [
        "net.dumm"
    ]
};

//IP address of host + cloudeebus port
//it should be updated
var IpAddrOfHost = "ws://172.27.0.174:9000";

/* DUMM DM Object Id*/
var objID = -1;

/* Variables purposed for html elements text modification. */
var stableSrc;
var progressSrc = 'img/ic_loader.gif';

/* Buttons */
var Buttons = {"Download" : 0, "Pause" : 1, "Abort" : 2, "None" : 3};
var buttonClicked = Buttons.None;

/* States */
var States = {"Idle" : 0, "Init" : 1, "Start" : 2, "Pause" : 3};
var State = States.Idle;

/* Helper method to load other javascript files.  */
function require(script) {
    $.ajax({
        url: script,
        dataType: "script",
        async: false,           // <-- this is the key
        success: function () {
            // all good...
        },
        error: function () {
            throw new Error("Could not load script " + script);
        }
    });
}

/* Set of methods modifying the buttons. */

function setProgressAnim(buttonPressed)
{
    var htmlObj;

    switch(buttonPressed){
    case Buttons.Download: 
    {
        htmlObj = $('inDownload');
        break;
    }
    case Buttons.Pause: 
    {
        htmlObj = $('inPause');
        break;
    }
    case Buttons.Abort:
    case Buttons.None: 
    {
        htmlObj = $('inAbort');
        break;
    }
    default: {
        alert("case default");
    }
    }
    stableSrc = htmlObj.getElementsByTagName("img")[0].src;
    htmlObj.getElementsByTagName("img")[0].src = progressSrc;
}

function setElemText(object, text)
{
    if (object.hasChildNodes()){
        object.removeChild(object.lastChild);
        object.appendChild(document.createTextNode(text));
    }
    else
        alert("Text cannot be set on selected object!");
}

function stopProgressAnim(object)
{
    var htmlObj;
    var text;

    switch(object){
    case Buttons.Download: 
    {
        htmlObj = $('inDownload');
        text = 'xxx';
        break;
    }
    case Buttons.Pause: 
    {
        htmlObj = $('inPause');
        text = 'xxx';
        break;
    }
    case Buttons.Abort:
    case Buttons.None: 
    {
        htmlObj = $('inAbort');
        text = 'xxx';
        break;
    }
    default: {
        alert("case default");
    }
    }

    htmlObj.getElementsByTagName("img")[0].src = stableSrc;
    //setElemText(htmlObj, text);
    buttonClicked = Buttons.None;
}

function onSuccStart() {
	console.log('Started downloading: '+objID);
	State = States.Start;
	setProgressAnim(Buttons.Download);

}

function onFailStart(error) {
	console.log('Error: Cannot start downloading');
	
	//it's better to remove object
	if(objID >= 0)
	{
		dumm.callMethod(dummIf,"Finalize",[objID], "u");
	}
	//TODO: should we change it in callback ?
	objID = -1;
	State = States.Idle;
}


function onSuccDown(id) {
	objID = id;
	console.log('Download object is created: '+id);
	State = States.Init;
	dumm.callMethod(dummIf,"Start",[objID], "u").then(onSuccStart, onFailStart);
}

function onFailDown(error) {
	console.log('Error: Cannot create download object');
	State = States.Idle;
}


function onSuccPause() {
	console.log('Download object is paused: '+objID);
	State = States.Pause;
	setProgressAnim(Buttons.Pause);	
}

function onFailPause(error) {
	console.log('Error: Cannot pause download object');
}

function onSuccRes() {
	console.log('Download object is resumed: '+objID);
	State = States.Start;
	stopProgressAnim(Buttons.Pause);	
}

function onFailRes(error) {
	console.log('Error: Cannot resume download object');
}

function onSuccAbort() {
	console.log('Download object is Abort: '+objID);
	State = States.Idle;
	dumm.callMethod(dummIf,"Finalize",[objID], "u");
	objID = -1;
	stopProgressAnim(Buttons.Download);
	stopProgressAnim(Buttons.Pause);	
}

function onFailAbort(error) {
	console.log('Error: Cannot Abort download object');
}


/* Define all actions on DOM Ready event. */
window.addEvent("domready", function() {

	//connect to DBus
    cloudeebus.connect(IpAddrOfHost, manifest, connected, errorCB);
	
    //buttons handlers below
    $('inPause').addEvent('click', function(){
    	
    	if(objID > -1)
    	{
    		if(State == States.Start)
    		{
    			dumm.callMethod(dummIf,"Pause",[objID], "u").then(onSuccPause, onFailPause);
    		}
    		else if(State == States.Pause)
    		{
    			dumm.callMethod(dummIf,"Resume",[objID], "u").then(onSuccRes, onFailRes);
    		}
    		else 
    		{
    			cloudeebus.log("Cannot pause - wrong state");
    		}
    	}
    });


    $('inDownload').addEvent('click', function(){
    	//new download
        if(objID == -1)
        {
            dumm.callMethod(dummIf,"Init",[$("in_url").value, '', '', 0, true, true, 'USER'], "sssibbs").then(onSuccDown, onFailDown);
        }
        else
        {
        	//we have some download object
        	if(State == States.Pause)
        	{
        		dumm.callMethod(dummIf,"Resume",[objID], "u").then(onSuccRes, onFailRes);
        	}
        }
    });
        
    $('inAbort').addEvent('click', function(){
    	
    	if(objID > -1)
    	{
    		if(State == States.Idle)
    		{
    			//well we should not have objID == -1
    			objID = -1;
    		}
    		else
    		{
    			dumm.callMethod(dummIf,"Abort",[objID], "u").then(onSuccAbort, onFailAbort);
    		}
    	}
    });
        
});

/* Helper function to debug clouddebus logs */
cloudeebus.log = function(msg)
{
    $('divLog').set('text', msg + "\n");
}

