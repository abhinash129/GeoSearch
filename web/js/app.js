
const API = "/api";

let allPlaces = [];
let currentPlaces = [];

let map;
let markerLayer;

let mapSearchTimer = null;

// Keeps recommendation markers from being overwritten
// by automatic map-window searches.
let showingRecommendations = false;

const elements = {
    serverStatus: document.getElementById("serverStatus"),
    totalPlaces: document.getElementById("totalPlaces"),
    matchingPlaces: document.getElementById("matchingPlaces"),
    totalCategories: document.getElementById("totalCategories"),

    keywordInput: document.getElementById("keywordInput"),
    categoryFilter: document.getElementById("categoryFilter"),
    ratingFilter: document.getElementById("ratingFilter"),
    priceFilter: document.getElementById("priceFilter"),

    radiusInput: document.getElementById("radiusInput"),
    recommendButton: document.getElementById("recommendButton"),

    placesList: document.getElementById("placesList"),
    resultMessage: document.getElementById("resultMessage"),
    resultCount: document.getElementById("resultCount")
};


// ------------------------------------
// API REQUEST
// ------------------------------------

async function apiRequest(endpoint) {

    const response = await fetch(`${API}${endpoint}`);

    const result = await response.json();

    if (!response.ok || !result.success) {
        throw new Error(result.error || "API request failed");
    }

    return result;
}


// ------------------------------------
// SERVER STATUS
// ------------------------------------

async function checkServer() {

    const statusDot = document.querySelector(".status-dot");

    try {

        await apiRequest("/health");

        elements.serverStatus.textContent = "Server Online";

        statusDot.classList.add("online");
        statusDot.classList.remove("offline");

    } catch (error) {

        elements.serverStatus.textContent = "Server Offline";

        statusDot.classList.add("offline");
        statusDot.classList.remove("online");
    }
}


// ------------------------------------
// INITIALIZE MAP
// ------------------------------------

function initializeMap() {

    map = L.map("map").setView(
        [28.62115, 77.36435],
        14
    );

    L.tileLayer(
        "https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png",
        {
            attribution: "&copy; OpenStreetMap contributors",
            maxZoom: 19
        }
    ).addTo(map);

    markerLayer = L.layerGroup().addTo(map);

    map.on("moveend", searchMapWindow);
}


// ------------------------------------
// LOAD ALL PLACES
// ------------------------------------

async function loadPlaces() {

    try {

        elements.placesList.textContent = "Loading places...";

        const result = await apiRequest("/places");

        allPlaces = result.data;

        elements.totalPlaces.textContent = allPlaces.length;

        populateCategories();

        currentPlaces = [...allPlaces];

        renderPlaces(currentPlaces);

        updateStatistics(currentPlaces);

    } catch (error) {

        showError(error.message);
    }
}


// ------------------------------------
// POPULATE CATEGORY FILTER
// ------------------------------------

function populateCategories() {

    const categories = [
        ...new Set(allPlaces.map(place => place.category))
    ].sort();

    elements.totalCategories.textContent = categories.length;

    elements.categoryFilter.replaceChildren();

    const defaultOption = document.createElement("option");

    defaultOption.value = "";
    defaultOption.textContent = "All Categories";

    elements.categoryFilter.appendChild(defaultOption);

    categories.forEach(category => {

        const option = document.createElement("option");

        option.value = category;
        option.textContent = category;

        elements.categoryFilter.appendChild(option);
    });
}


// ------------------------------------
// FORMAT OPTIONAL PLACE INFORMATION
// ------------------------------------

function formatRating(value) {

    if (
        value === null ||
        value === undefined ||
        value === "" ||
        !Number.isFinite(Number(value)) ||
        Number(value) < 0 ||
        Number(value) > 5
    ) {
        return "Not available";
    }

    return `${Number(value).toFixed(1)} / 5`;
}

function formatPrice(value) {

    if (
        value === null ||
        value === undefined ||
        value === "" ||
        !Number.isFinite(Number(value)) ||
        Number(value) < 0
    ) {
        return "Not available";
    }

    return Number(value).toFixed(2);
}


// ------------------------------------
// RENDER PLACE CARDS
// ------------------------------------

function renderPlaces(places) {

    elements.placesList.replaceChildren();

    if (places.length === 0) {

        const message = document.createElement("p");

        message.className = "empty-message";
        message.textContent = "No matching places found.";

        elements.placesList.appendChild(message);

        elements.resultCount.textContent = "0 results";

        updateStatistics([]);

        updateMapMarkers([]);

        return;
    }

    places.forEach(place => {

        const card = document.createElement("article");

        card.className = "place-card";

        const title = document.createElement("h3");
        title.textContent = place.name;

        const description = document.createElement("p");
        description.textContent = place.description;

        const meta = document.createElement("div");
        meta.className = "place-meta";

        const category = createTag(place.category);

        const rating = createTag(
            `Rating: ${formatRating(place.rating)}`,
            "rating-tag"
        );

        const price = createTag(
            `Price: ${formatPrice(place.price)}`
        );

        const location = createTag(
            `${place.latitude}, ${place.longitude}`
        );

        meta.append(category, rating, price, location);

        // Display distance when available
        if (place.distanceKm !== undefined) {

            const distance = createTag(
                `Distance: ${place.distanceKm.toFixed(3)} km`
            );

            meta.appendChild(distance);
        }

        // Display recommendation score when available
        if (place.recommendationScore !== undefined) {

            const recommendation = createTag(
                `Recommendation: ${(place.recommendationScore * 100).toFixed(1)}%`,
                "rating-tag"
            );

            meta.appendChild(recommendation);
        }

        card.append(title, description, meta);

        const details = createPlaceDetails(place);

        if (details) {
            card.appendChild(details);
        }

        card.addEventListener("click", () => {

            map.setView(
                [place.latitude, place.longitude],
                17
            );

        });

        elements.placesList.appendChild(card);
    });

    elements.resultCount.textContent = `${places.length} results`;

    updateStatistics(places);

    updateMapMarkers(places);
}


// ------------------------------------
// CREATE TAG
// ------------------------------------

function createTag(value, extraClass = "") {

    const tag = document.createElement("span");

    tag.className = `meta-tag ${extraClass}`;
    tag.textContent = value;

    return tag;
}


// ------------------------------------
// OPTIONAL PLACE DETAILS
// ------------------------------------

function getSafeWebsiteUrl(value) {

    if (!value || typeof value !== "string") {
        return null;
    }

    try {
        const url = new URL(value);

        if (url.protocol !== "http:" && url.protocol !== "https:") {
            return null;
        }

        return url.href;

    } catch {
        return null;
    }
}

function createPlaceDetails(place) {

    const fields = [
        ["Address", place.address],
        ["Cuisine", place.cuisine],
        ["Opening hours", place.openingHours],
        ["Phone", place.phone],
        ["Brand", place.brand],
        ["Operator", place.operator]
    ];

    const details = document.createElement("div");
    details.className = "place-details";

    fields.forEach(([label, value]) => {

        if (typeof value !== "string" || !value.trim()) {
            return;
        }

        const row = document.createElement("p");
        const heading = document.createElement("strong");

        heading.textContent = `${label}: `;
        row.appendChild(heading);
        row.appendChild(document.createTextNode(value));

        details.appendChild(row);
    });

    const websiteUrl = getSafeWebsiteUrl(place.website);

    if (websiteUrl) {

        const row = document.createElement("p");
        const heading = document.createElement("strong");
        const link = document.createElement("a");

        heading.textContent = "Website: ";
        link.href = websiteUrl;
        link.textContent = place.website;
        link.target = "_blank";
        link.rel = "noopener noreferrer";

        row.append(heading, link);
        details.appendChild(row);
    }

    return details.childElementCount > 0 ? details : null;
}


// ------------------------------------
// UPDATE STATISTICS
// ------------------------------------

function updateStatistics(places) {

    elements.matchingPlaces.textContent = places.length;

    elements.resultMessage.textContent =
        `Showing ${places.length} matching places`;
}


// ------------------------------------
// SEARCH BY KEYWORD
// ------------------------------------

async function searchKeyword() {

    showingRecommendations = false;

    const keyword = elements.keywordInput.value.trim();

    if (!keyword) {

        await applyFilters();

        return;
    }

    try {

        const result = await apiRequest(
            `/search?keyword=${encodeURIComponent(keyword)}`
        );

        let places = result.data;

        places = applyLocalFilters(places);

        currentPlaces = places;

        renderPlaces(places);

        elements.resultMessage.textContent =
            `Keyword results for "${keyword}"`;

    } catch (error) {

        showError(error.message);
    }
}


// ------------------------------------
// APPLY FILTERS
// ------------------------------------

async function applyFilters() {

    showingRecommendations = false;

    try {

        let places = [...allPlaces];

        const keyword = elements.keywordInput.value.trim();

        const category = elements.categoryFilter.value;

        if (keyword) {

            const result = await apiRequest(
                `/search?keyword=${encodeURIComponent(keyword)}`
            );

            places = result.data;
        }

        if (category) {

            const result = await apiRequest(
                `/category?name=${encodeURIComponent(category)}`
            );

            const matchingIds = new Set(
                result.data.map(place => place.id)
            );

            places = places.filter(
                place => matchingIds.has(place.id)
            );
        }

        places = applyLocalFilters(places);

        currentPlaces = places;

        renderPlaces(places);

        elements.resultMessage.textContent = "Search filters applied";

    } catch (error) {

        showError(error.message);
    }
}


// ------------------------------------
// LOCAL FILTERS
// ------------------------------------

function applyLocalFilters(places) {

    const minimumRating = Number(elements.ratingFilter.value);

    const maximumPrice =
        elements.priceFilter.value === ""
            ? Infinity
            : Number(elements.priceFilter.value);

    return places.filter(place => {

        return place.rating >= minimumRating &&
            place.price <= maximumPrice;
    });
}


// ------------------------------------
// TOP-RATED PLACES
// ------------------------------------

async function loadTopRated() {

    showingRecommendations = false;

    try {

        const result = await apiRequest("/top-rated?k=5");

        currentPlaces = result.data;

        renderPlaces(currentPlaces);

        elements.resultMessage.textContent =
            "Top 5 highest-rated places";

    } catch (error) {

        showError(error.message);
    }
}


// ------------------------------------
// NEAREST PLACES
// ------------------------------------

async function findNearest() {

    showingRecommendations = false;

    if (!navigator.geolocation) {

        alert("Geolocation is not supported by your browser.");

        return;
    }

    navigator.geolocation.getCurrentPosition(

        async position => {

            try {

                const latitude = position.coords.latitude;
                const longitude = position.coords.longitude;

                const result = await apiRequest(
                    `/nearest?lat=${latitude}&lon=${longitude}&k=5`
                );

                currentPlaces = result.data.map(item => ({
                    ...item.place,
                    distanceKm: item.distanceKm
                }));

                renderPlaces(currentPlaces);

                map.setView(
                    [latitude, longitude],
                    14
                );

                elements.resultMessage.textContent =
                    "Five nearest places to your location";

            } catch (error) {

                showError(error.message);
            }
        },

        error => {

            alert(
                "Unable to access your location. " +
                "Please allow location permission in your browser."
            );
        }
    );
}


// ------------------------------------
// SMART RECOMMENDATIONS
// ------------------------------------

async function findBestPlaces() {

    if (!navigator.geolocation) {

        alert("Geolocation is not supported by your browser.");

        return;
    }

    // Read search radius
    const radius = Number(elements.radiusInput.value);

    if (!Number.isFinite(radius) || radius <= 0 || radius > 1000) {

        alert("Please enter a valid search radius between 0.1 and 1000 km.");

        return;
    }

    // Prevent repeated clicks while searching
    elements.recommendButton.disabled = true;

    elements.recommendButton.textContent = "Finding Best Places...";

    navigator.geolocation.getCurrentPosition(

        async position => {

            try {

                const latitude = position.coords.latitude;
                const longitude = position.coords.longitude;

                // Read selected category
                const category = elements.categoryFilter.value;

                // Prepare API parameters
                const params = new URLSearchParams({

                    lat: latitude,
                    lon: longitude,
                    radius: radius,
                    limit: 5

                });

                if (category) {

                    params.set("category", category);
                }

                // Call C++ recommendation API
                const result = await apiRequest(
                    `/recommend?${params.toString()}`
                );

                // Convert API response into place objects
                currentPlaces = result.data.map(item => ({

                    ...item.place,

                    distanceKm: item.distanceKm,

                    recommendationScore: item.overallScore,

                    distanceScore: item.distanceScore,

                    ratingScore: item.ratingScore,

                    priceScore: item.priceScore

                }));

                // Preserve recommendation markers
                showingRecommendations = true;

                // Display ranked recommendations
                renderPlaces(currentPlaces);

                // Center map around user's location
                map.setView(
                    [latitude, longitude],
                    14
                );

                // Update result message
                elements.resultMessage.textContent =
                    `Best ${category || "nearby"} places within ${radius} km, ranked by distance, rating and price`;

                if (currentPlaces.length === 0) {

                    elements.resultMessage.textContent =
                        `No places found within ${radius} km. Try increasing the search radius.`;
                }

            } catch (error) {

                showError(error.message);
            }

            finally {

                elements.recommendButton.disabled = false;

                elements.recommendButton.textContent = "Find Best Places";
            }
        },

        error => {

            elements.recommendButton.disabled = false;

            elements.recommendButton.textContent = "Find Best Places";

            let message = "Unable to access your location.";

            if (error.code === error.PERMISSION_DENIED) {

                message += " Please allow location permission.";

            } else if (error.code === error.POSITION_UNAVAILABLE) {

                message += " Your location is currently unavailable.";

            } else if (error.code === error.TIMEOUT) {

                message += " Location request timed out.";
            }

            alert(message);
        },

        {
            enableHighAccuracy: true,
            timeout: 15000,
            maximumAge: 0
        }
    );
}


// ------------------------------------
// MAP MARKERS
// ------------------------------------

function updateMapMarkers(places) {

    if (!markerLayer) {
        return;
    }

    markerLayer.clearLayers();

    places.forEach(place => {

        const marker = L.marker([
            place.latitude,
            place.longitude
        ]);

        let popupContent = `
            <strong>${escapeHTML(place.name)}</strong><br>
            Category: ${escapeHTML(place.category)}<br>
            Rating: ${escapeHTML(formatRating(place.rating))}<br>
            Price: ${escapeHTML(formatPrice(place.price))}
        `;

        const popupFields = [
            ["Address", place.address],
            ["Cuisine", place.cuisine],
            ["Opening hours", place.openingHours],
            ["Phone", place.phone],
            ["Brand", place.brand],
            ["Operator", place.operator]
        ];

        popupFields.forEach(([label, value]) => {

            if (typeof value === "string" && value.trim()) {
                popupContent +=
                    `<br>${escapeHTML(label)}: ${escapeHTML(value)}`;
            }
        });

        const websiteUrl = getSafeWebsiteUrl(place.website);

        if (websiteUrl) {
            popupContent +=
                `<br>Website: <a href="${escapeHTML(websiteUrl)}" target="_blank" rel="noopener noreferrer">${escapeHTML(place.website)}</a>`;
        }

        if (place.distanceKm !== undefined) {

            popupContent +=
                `<br>Distance: ${place.distanceKm.toFixed(3)} km`;
        }

        if (place.recommendationScore !== undefined) {

            popupContent +=
                `<br>Recommendation: ${(place.recommendationScore * 100).toFixed(1)}%`;
        }

        marker.bindPopup(popupContent);

        marker.addTo(markerLayer);
    });
}


// ------------------------------------
// HTML ESCAPE
// ------------------------------------

function escapeHTML(value) {

    return String(value).replace(/[&<>"']/g, character => {

        const replacements = {

            "&": "&amp;",
            "<": "&lt;",
            ">": "&gt;",
            '"': "&quot;",
            "'": "&#39;"

        };

        return replacements[character];
    });
}


// ------------------------------------
// MAP WINDOW SEARCH
// ------------------------------------

async function searchMapWindow() {

    if (!map || showingRecommendations) {
        return;
    }

    clearTimeout(mapSearchTimer);

    mapSearchTimer = setTimeout(async () => {

        try {

            const bounds = map.getBounds();

            const params = new URLSearchParams({

                minLat: bounds.getSouth(),
                maxLat: bounds.getNorth(),

                minLon: bounds.getWest(),
                maxLon: bounds.getEast(),

                minRating: elements.ratingFilter.value

            });

            const result = await apiRequest(
                `/map-window?${params.toString()}`
            );

            updateMapMarkers(result.data);

        } catch (error) {

            console.error("Map search failed:", error);
        }

    }, 350);
}


// ------------------------------------
// RESET FILTERS
// ------------------------------------

function resetFilters() {

    showingRecommendations = false;

    elements.keywordInput.value = "";

    elements.categoryFilter.value = "";

    elements.ratingFilter.value = "0";

    elements.priceFilter.value = "";

    elements.radiusInput.value = "5";

    currentPlaces = [...allPlaces];

    renderPlaces(currentPlaces);

    elements.resultMessage.textContent = "All places";
}


// ------------------------------------
// ERROR DISPLAY
// ------------------------------------

function showError(message) {

    elements.placesList.replaceChildren();

    const error = document.createElement("p");

    error.className = "empty-message";

    error.textContent = `Error: ${message}`;

    elements.placesList.appendChild(error);

    elements.resultMessage.textContent = "Unable to load results";
}


// ------------------------------------
// EVENT LISTENERS
// ------------------------------------

document.getElementById("searchButton")
    .addEventListener("click", searchKeyword);


elements.keywordInput
    .addEventListener("keydown", event => {

        if (event.key === "Enter") {

            searchKeyword();
        }
    });


document.getElementById("applyFilters")
    .addEventListener("click", applyFilters);


document.getElementById("resetFilters")
    .addEventListener("click", resetFilters);


document.getElementById("topRatedButton")
    .addEventListener("click", loadTopRated);


document.getElementById("nearestButton")
    .addEventListener("click", findNearest);


document.getElementById("allPlacesButton")
    .addEventListener("click", resetFilters);


// Smart recommendation button
elements.recommendButton
    .addEventListener("click", findBestPlaces);


// ------------------------------------
// APPLICATION STARTUP
// ------------------------------------

async function initializeApplication() {

    initializeMap();

    await checkServer();

    await loadPlaces();

    searchMapWindow();
}

initializeApplication();
