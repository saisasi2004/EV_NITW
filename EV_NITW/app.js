//
//    This code is patented.
//    © 2025 Gampa Sai Sasivardhan. All rights reserved.
//    Unauthorized reproduction, distribution, or modification is prohibited.
//

var firebaseConfig = {
    apiKey: "AIzaSyCfTYSpYMgwWyECubaiToR5Chm9FUWAHTg",
    authDomain: "evnitw.firebaseapp.com",
    databaseURL: "https://evnitw-default-rtdb.firebaseio.com",
    projectId: "evnitw",
    storageBucket: "evnitw.appspot.com",
    messagingSenderId: "504679121631",
    appId: "1:504679121631:web:50318aa2ebbdeb0d55df6a",
    measurementId: "G-SPXZBNN0F9"
};

firebase.initializeApp(firebaseConfig);

var coordinatesRefs = {
    EV1: firebase.database().ref("/coordinatesEV1"),
    EV2: firebase.database().ref("/coordinatesEV2"),
    EV3: firebase.database().ref("/coordinatesEV3")
};

var seatCountRefs = {
    EV1: firebase.database().ref("/seatCountEV1"),
    EV2: firebase.database().ref("/seatCountEV2"),
    EV3: firebase.database().ref("/seatCountEV3")
};

var stopRequestRef = firebase.database().ref("/stopRequests");
var emergencyStopRequestRef = firebase.database().ref("/emergencyStopRequests"); 

var seatCounts = { EV1: 0, EV2: 0, EV3: 0 }; // Initial seat counts
var map, pushpins = {};
var autoCenterMap = true;
var stopRequestsQueue = []; // Array to store stop locations
var isPlaying = false; // Flag to check if audio is playing

function loadMapScenario() {
    map = new Microsoft.Maps.Map(document.getElementById('myMap'), {
        center: new Microsoft.Maps.Location(17.9856, 79.5308),
        zoom: 16,
        mapTypeId: Microsoft.Maps.MapTypeId.road
    });

    createPushpin("EV1", 17.98818, 79.5308);
    createPushpin("EV2", 17.98801, 79.5305);
    createPushpin("EV3", 17.9878, 79.5308);

    setupFirebaseListeners();
    setupSeatCountListeners();  // Set up Firebase listeners for seat count
}

function createPushpin(evId, latitude, longitude) {
    var icon = 'car.png';

    pushpins[evId] = new Microsoft.Maps.Pushpin(new Microsoft.Maps.Location(latitude, longitude), {
        title: evId,
        icon: icon
    });

    // Create an Infobox to display the seat count badge
    var badgeInfobox = new Microsoft.Maps.Infobox(pushpins[evId].getLocation(), {
        visible: false,
        offset: new Microsoft.Maps.Point(0, 20), // Adjust offset as needed
        description: `${evId} - ${seatCounts[evId]} Seats`
    });

    // Store both the pushpin and badgeInfobox in the pushpins object
    pushpins[evId].metadata = { badgeInfobox: badgeInfobox };
    map.entities.push(pushpins[evId]);
    map.entities.push(badgeInfobox);

    // Show the badgeInfobox
    badgeInfobox.setOptions({ visible: true });
}

function updateLocation(evId, latitude, longitude) {
    if (pushpins[evId]) {
        var newLocation = new Microsoft.Maps.Location(latitude, longitude);
        pushpins[evId].setLocation(newLocation);
        pushpins[evId].metadata.badgeInfobox.setLocation(newLocation);
    }
}

function updateBadge(evId, seatCount) {
    if (pushpins[evId] && pushpins[evId].metadata.badgeInfobox) {
        seatCounts[evId] = seatCount; // Update seat count
        pushpins[evId].metadata.badgeInfobox.setOptions({
            description: `${evId} - ${seatCount}/8 People`,
            visible: true
        });
    }
}

function setupFirebaseListeners() {
    coordinatesRefs.EV1.on('value', function(snapshot) {
        var data = snapshot.val();
        if (data && data.latitude && data.longitude) {
            updateLocation("EV1", data.latitude, data.longitude);
        } else {
            console.error("No data for EV1");
        }
    });

    coordinatesRefs.EV2.on('value', function(snapshot) {
        var data = snapshot.val();
        if (data && data.latitude && data.longitude) {
            updateLocation("EV2", data.latitude, data.longitude);
        } else {
            console.error("No data for EV2");
        }
    });

    coordinatesRefs.EV3.on('value', function(snapshot) {
        var data = snapshot.val();
        if (data && data.latitude && data.longitude) {
            updateLocation("EV3", data.latitude, data.longitude);
        } else {
            console.error("No data for EV3");
        }
    });
}

function setupSeatCountListeners() {
    seatCountRefs.EV1.on('value', function(snapshot) {
        var seatCount = snapshot.val();
        if (seatCount !== null) {
            updateBadge("EV1", seatCount); // Update the badge on the map
        }
    });

    seatCountRefs.EV2.on('value', function(snapshot) {
        var seatCount = snapshot.val();
        if (seatCount !== null) {
            updateBadge("EV2", seatCount); // Update the badge on the map
        }
    });

    seatCountRefs.EV3.on('value', function(snapshot) {
        var seatCount = snapshot.val();
        if (seatCount !== null) {
            updateBadge("EV3", seatCount); // Update the badge on the map
        }
    });
}

function requestStop() {
    var stopLocation = document.getElementById('stopDropdown').value;
    if (stopLocation !== "") {
        stopRequestRef.set({
            stopLocation: stopLocation,
        }, function(error) {
            if (error) {
                console.error("Error sending stop request:", error);
            } else {
                alert("Stop request sent successfully: " + stopLocation);
                enqueueStopRequest(stopLocation);
            }
        });
    } else {
        alert("Please select a stop location.");
    }
}

function emergencyRequestStop() {
    var stopLocation = document.getElementById('EmergencyDropdown').value;
    if (stopLocation !== "") {
        emergencyStopRequestRef.set({
            stopLocation: stopLocation,
        }, function(error) {
            if (error) {
                console.error("Error sending emergency stop request:", error);
            } else {
                alert("Emergency Stop request sent successfully: " + stopLocation);

                playEmergencyStopRequestSound(stopLocation);

                setTimeout(function() {
                    emergencyStopRequestRef.set({
                        stopLocation: ""
                    }, function(error) {
                        if (error) {
                            console.error("Error clearing emergency stop request:", error);
                        } else {
                            console.log("Emergency Stop request cleared after 4 seconds.");
                        }
                    });
                }, 4000);
            }
        });
    } else {
        alert("Please select a stop location.");
    }
}

function enqueueStopRequest(stopLocation) {
    stopRequestsQueue.push(stopLocation); // Add location to the queue
    console.log(stopRequestsQueue);
    if (!isPlaying) {
        playNextStopRequest();
    }
}

function playNextStopRequest() {
    if (stopRequestsQueue.length === 0) {
        isPlaying = false;
        return;
    }
    
    isPlaying = true;
    var currentLocation = stopRequestsQueue.shift(); // Get the next location
    playStopRequestSound(currentLocation);

    setTimeout(function() {
        clearStopRequest();
        playNextStopRequest();
    }, 4000); // Wait 4 seconds before playing the next sound
}

function clearStopRequest() {
    stopRequestRef.set({
        stopLocation: ""
    }, function(error) {
        if (error) {
            console.error("Error clearing stop request:", error);
        } else {
            console.log("Stop request cleared.");
        }
    });
}

function playStopRequestSound(stopLocation) {
    console.log("Playing sound for stop location:", stopLocation);
}

function playEmergencyStopRequestSound(stopLocation) {
    console.log("Playing emergency sound for stop location:", stopLocation);
}

function EVtimingsPDF() {
    window.open('EV_NITW.pdf', 'EV_NITW.pdf'); 
}

function toggleAutoCenter() {
    autoCenterMap = !autoCenterMap;
    document.getElementById('autoCenterStatus').innerText = autoCenterMap ? 'Enabled' : 'Disabled';
}

function showEmergencyPopup() {
    document.getElementById('emergencyPopup').style.display = 'flex';
}

function closePopup() {
    document.getElementById('emergencyPopup').style.display = 'none';
}

window.onload = loadMapScenario;