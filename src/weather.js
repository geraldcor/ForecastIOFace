var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function getResourceIdForIconString(icon) {
  // Build icon resource id from forecast.io text
  var icon_resource_ids = {
    'clear-day': 0,
    'clear-night': 1,
    'rain': 2,
    'snow': 3,
    'sleet': 4,
    'wind': 5,
    'fog': 6,
    'cloudy': 7,
    'partly-cloudy-day': 8,
    'partly-cloudy-night': 9,
    'hail': 10,
    'thunderstorm': 11
  };
  // Make sure we have a default resource id
  var resource_id = 0;
  try {
    resource_id = icon_resource_ids[icon];  
  } catch (error) {
    console.log("Unrecognized Icon Name");
  }
  return resource_id;
}

function locationSuccess(pos) {
  // Construct URL
  var url = 'https://api.forecast.io/forecast/acb79d16706f871691877ca0e5a9f346/' +
      pos.coords.latitude + ',' + pos.coords.longitude;

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      console.log(json);
      // Temperature in Kelvin requires adjustment
      var temperature = Math.round(json.currently.temperature);
      console.log('Temperature is ' + temperature);

      // Conditions
      var conditions = json.minutely.summary;
      var icon = json.minutely.icon;
      // Day Summary
      var day_summary = json.hourly.summary;
      console.log('Conditions are ' + conditions + " " + icon + " " + day_summary);
      
      // Get a valid resource id from the icon string
      var resource_id = getResourceIdForIconString(icon);
      console.log("RESOURCE ID FROM JS IS " + resource_id);
      // Assemble dictionary using our keys
      var dictionary = {
        'KEY_TEMPERATURE': temperature,
        'KEY_CONDITIONS': conditions,
        'KEY_ICON': resource_id,
        'KEY_DAY_SUMMARY': day_summary
      };
      
      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log('Weather info sent to Pebble successfully!');
        },
        function(e) {
          console.log('Error sending weather info to Pebble!');
        }
      );
    }      
  );
}

function locationError(err) {
  console.log('Error requesting location!');
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
    console.log('PebbleKit JS ready!');

    // Get the initial weather
    getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    getWeather();
  }                     
);