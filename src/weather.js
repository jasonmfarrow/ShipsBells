
var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

var options = {};

function locationSuccess(pos) {
  // We will request the weather here
  var url = "http://api.openweathermap.org/data/2.5/weather?lat=" +
  pos.coords.latitude + "&lon=" + pos.coords.longitude;
  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      console.log("full response:" + responseText);

      // Temperature in Kelvin requires adjustment
      var temperature = Math.round(json.main.temp - 273.15);
      console.log("Temperature is " + temperature);

      // Conditions
      var conditions = json.weather[0].main;      
      console.log("Conditions are " + conditions);
      // Name
      var name = json.name;      
      console.log("Name is " + name);
      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_TEMPERATURE": temperature,
        "KEY_LOCATION": name,
        "KEY_CONDITIONS": conditions
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
    }      
  );
}

function locationError(err) {
  console.log("Error requesting location!");
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready! Setting initial weather.");

    // Get the initial weather
    getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received! Updating weather.");
    getWeather();
  }                     
);

Pebble.addEventListener("showConfiguration", function() {
  console.log("Showing configuration");
  Pebble.openURL('http://jasonmfarrow.com/ships-bells-config-page.html');
});

Pebble.addEventListener("webviewclosed", function(e) {
  console.log("Configuration closed");
  //Get JSON dictionary
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log("Configuration window returned: " + JSON.stringify(configuration));
      //Send to Pebble, persist there
        // Assemble dictionary using our keys
      var dictionary = {
        "BLACK_ON_WHITE": configuration.BLACK_ON_WHITE,
        "HOUR_BELLS_FROM": configuration.HOUR_BELLS_FROM,
        "HOUR_BELLS_TO": configuration.HOUR_BELLS_TO,
        "HALF_HOUR_BELLS": configuration.HALF_HOUR_BELLS,
        "QUARTER_HOUR_BELLS": configuration.QUARTER_HOUR_BELLS
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
});