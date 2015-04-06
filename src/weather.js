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
    'thunderstorm': 11,
    'clear-day-small': 12,
    'clear-night-small': 13,
    'rain-small': 14,
    'snow-small': 15,
    'sleet-small': 16,
    'wind-small': 17,
    'fog-small': 18,
    'cloudy-small': 19,
    'partly-cloudy-day-small': 20,
    'partly-cloudy-night-small': 21,
    'hail-small': 22,
    'thunderstorm-small': 23
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
  console.log(url);
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
      // Days Summary
      var daily = json.daily.data;
      var day_1_day = new Date(daily[1].time * 1000).toDateString().split(" ")[0];
      var day_1_temp = Math.round(daily[1].temperatureMax);
      var day_1_icon = daily[1].icon + '-small';
      var day_2_day = new Date(daily[2].time * 1000).toDateString().split(" ")[0];
      var day_2_temp = Math.round(daily[2].temperatureMax);
      var day_2_icon = daily[2].icon + '-small';
      console.log("Days are " + day_1_day + " :: " + day_2_day);
      // Get a valid resource id from the icon string
      var resource_id = getResourceIdForIconString(icon);
      var day_1_resource_id = getResourceIdForIconString(day_1_icon);
      var day_2_resource_id = getResourceIdForIconString(day_2_icon);
      
      // Assemble dictionary using our keys
      var dictionary = {
        'KEY_TEMPERATURE': temperature,
        'KEY_CONDITIONS': conditions,
        'KEY_ICON': resource_id,
        'KEY_DAY_ONE_TEMPERATURE': day_1_temp,
        'KEY_DAY_ONE_ICON': day_1_resource_id,
        'KEY_DAY_TWO_TEMPERATURE': day_2_temp,
        'KEY_DAY_TWO_ICON': day_2_resource_id,
        'KEY_DAY_ONE_DAY': day_1_day,
        'KEY_DAY_TWO_DAY': day_2_day
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