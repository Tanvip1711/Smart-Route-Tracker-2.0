
// ==========================================
// SMART ROUTE TRACKER 2.0
// Frontend + C++ API Integration
// ==========================================


// 1. INITIALISE LEAFLET MAP

const map = L.map('map').setView([18.5204, 73.8567], 13);

L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution: '&copy; OpenStreetMap contributors'
}).addTo(map);

let routeLayer = null;
let sourceMarker = null;
let destinationMarker = null;

const initialMarker = L.marker([18.5204, 73.8567])
    .addTo(map)
    .bindPopup('Pune');


// 2. GET HTML ELEMENTS

const sourceInput = document.getElementById('source');
const destinationInput = document.getElementById('destination');
const algorithmInput = document.getElementById('algorithm');
const findRouteButton = document.getElementById('find-route-btn');

const currentLocationButton =
    document.getElementById('use-current-location-btn');

const message = document.getElementById('message');
const distance = document.getElementById('distance');
const eta = document.getElementById('eta');
const routeStatus = document.getElementById('route-status');

let gpsSource = null;


// GPS BUTTON

sourceInput.addEventListener('input', () => {
    gpsSource = null;
});

currentLocationButton.addEventListener('click', () => {

    if (!navigator.geolocation) {
        message.textContent = 'Geolocation is not supported by your browser.';
        return;
    }

    currentLocationButton.disabled = true;
    message.textContent = 'Detecting your current location...';

    navigator.geolocation.getCurrentPosition(

        (position) => {

            gpsSource = {
                latitude: position.coords.latitude,
                longitude: position.coords.longitude,
                displayName: 'Current Location'
            };

            sourceInput.value = 'Current Location';

            message.textContent =
                'Location detected! Enter your destination.';

            currentLocationButton.disabled = false;
        },

        (error) => {

            message.textContent = error.code === 1
                ? 'Location permission denied. Please allow location access.'
                : 'Unable to detect location. Please try again.';

            currentLocationButton.disabled = false;
        },

        {
    enableHighAccuracy: false,
    timeout: 20000,
    maximumAge: 300000
}
    );
});

// 3. CHECK BACKEND CONNECTION

async function checkBackend() {

    try {

        const response = await fetch('http://localhost:8080/health');

        if (!response.ok) {
            throw new Error('Backend response was not successful');
        }

        const data = await response.json();

        console.log('Backend connected:', data);

        message.textContent =
            'Backend connected successfully. Enter your locations.';

    } catch (error) {

        console.error('Backend connection failed:', error);

        message.textContent =
            'Backend connection unavailable. Please start the C++ API.';
    }
}

checkBackend();


// 4. GEOCODE LOCATION NAME USING OPENSTREETMAP

async function geocodeLocation(placeName) {

    const searchQuery =
        placeName + ', Pune, Maharashtra, India';

    const url =
        'https://nominatim.openstreetmap.org/search?format=jsonv2&limit=1&countrycodes=in&q='
        + encodeURIComponent(searchQuery);

    const response = await fetch(url, {
        headers: {
            'Accept': 'application/json'
        }
    });

    if (!response.ok) {
        throw new Error('Location search service is unavailable.');
    }

    const results = await response.json();

    if (!results.length) {

        throw new Error(
            'Could not find "' + placeName +
            '". Try a more specific location.'
        );
    }

    return {
        latitude: parseFloat(results[0].lat),
        longitude: parseFloat(results[0].lon),
        displayName: results[0].display_name
    };
}


// 5. WAIT BETWEEN GEOCODING REQUESTS

function wait(milliseconds) {

    return new Promise(resolve => setTimeout(resolve, milliseconds));

}


// 6. REQUEST ROUTE FROM C++ BACKEND

async function calculateRoute(source, destination) {

    // Read the user's selected route preference

    const selectedAlgorithm = algorithmInput.value;

    // Best Route currently uses the fastest-route algorithm

    const backendAlgorithm =
        selectedAlgorithm === 'best'
            ? 'fastest'
            : selectedAlgorithm;

    const response = await fetch('http://localhost:8080/route', {

        method: 'POST',

        headers: {
            'Content-Type': 'application/json'
        },

        body: JSON.stringify({

            source: {
                latitude: source.latitude,
                longitude: source.longitude
            },

            destination: {
                latitude: destination.latitude,
                longitude: destination.longitude
            },

            algorithm: backendAlgorithm

        })

    });

    const result = await response.json();

    if (!response.ok) {

        throw new Error(
            result.error || 'Route calculation failed.'
        );
    }

    return result;
}


// 7. DISPLAY ROUTE ON LEAFLET MAP

function displayRoute(result, source, destination) {

    if (!result.path || result.path.length === 0) {

        throw new Error('The backend returned an empty route.');

    }


    // Remove previous route and markers

    if (routeLayer) {
        map.removeLayer(routeLayer);
    }

    if (sourceMarker) {
        map.removeLayer(sourceMarker);
    }

    if (destinationMarker) {
        map.removeLayer(destinationMarker);
    }

    map.removeLayer(initialMarker);


    // Convert backend path into Leaflet coordinates

    const routeCoordinates = result.path.map(node => [

        node.latitude,
        node.longitude

    ]);


    // Connect route visually to exact destination marker

    routeCoordinates.push([

        destination.latitude,
        destination.longitude

    ]);


    // Draw calculated road route

    routeLayer = L.polyline(routeCoordinates, {

        color: '#2563eb',
        weight: 6,
        opacity: 0.85

    }).addTo(map);


    // Add starting point marker

    sourceMarker = L.marker([

        source.latitude,
        source.longitude

    ])
    .addTo(map)
    .bindPopup('Starting Point');


    // Add destination marker

    destinationMarker = L.marker([

        destination.latitude,
        destination.longitude

    ])
    .addTo(map)
    .bindPopup('Destination');


    // Zoom to fit entire route

    map.fitBounds(routeLayer.getBounds(), {

        padding: [40, 40]

    });


    // Update journey overview

    distance.textContent =
        Number(result.distance_km).toFixed(2) + ' km';

    eta.textContent =
        Number(result.travel_time_minutes).toFixed(1) + ' min';

    routeStatus.textContent = 'Route found';


    // Display a user-friendly message

    const routeNames = {

        shortest: 'Shortest Route',
        fastest: 'Fastest Route',
        astar: 'Best Route'

    };

    const selectedRouteName =
        algorithmInput.value === 'best'
            ? 'Best Route'
            : routeNames[algorithmInput.value];


    message.textContent =
        selectedRouteName +
        ' calculated successfully. Live traffic data is unavailable.';


    console.log('Route result:', result);

}


// 8. HANDLE FIND ROUTE BUTTON

findRouteButton.addEventListener('click', async function () {

    const sourceName = sourceInput.value.trim();
    const destinationName = destinationInput.value.trim();


    // Validate input fields

    if (!sourceName || !destinationName) {

        message.textContent =
            'Please enter both starting point and destination.';

        return;

    }


    try {

        findRouteButton.disabled = true;

        message.textContent = 'Searching for starting location...';

        distance.textContent = '-- km';
        eta.textContent = '-- min';
        routeStatus.textContent = 'Calculating';


        // Search source location

const source = gpsSource || await geocodeLocation(sourceName);

        // Respect Nominatim's public rate limit

        message.textContent = 'Searching for destination...';

        await wait(1100);


        // Search destination location

        const destination = await geocodeLocation(destinationName);


        // Request route from C++ API

        message.textContent =
            'Calculating your selected route...';

        const result = await calculateRoute(source, destination);


        // Display route on map

        displayRoute(result, source, destination);


    } catch (error) {

        console.error('Route error:', error);

        message.textContent = error.message;

        routeStatus.textContent = 'Failed';


    } finally {

        findRouteButton.disabled = false;

    }

});


// 9. MAP CLICK INTERACTION

map.on('click', function (event) {

    const latitude = event.latlng.lat.toFixed(6);
    const longitude = event.latlng.lng.toFixed(6);

    console.log('Selected map location:', latitude, longitude);

});
