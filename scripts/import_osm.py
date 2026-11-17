import argparse
import csv
import json
import time
import urllib.parse
import urllib.request
from pathlib import Path

DEFAULT_LAT = 28.62115
DEFAULT_LON = 77.36435
DEFAULT_RADIUS = 5000

ENDPOINTS = [
    "https://overpass-api.de/api/interpreter",
    "https://overpass.kumi.systems/api/interpreter",
]

AMENITY_CATEGORIES = {
    "hospital": "Hospital",
    "clinic": "Clinic",
    "doctors": "Clinic",
    "dentist": "Dental Clinic",
    "pharmacy": "Pharmacy",
    "veterinary": "Veterinary Clinic",
    "restaurant": "Restaurant",
    "cafe": "Cafe",
    "fast_food": "Restaurant",
    "food_court": "Food Court",
    "bank": "Bank",
    "atm": "ATM",
    "fuel": "Fuel Station",
    "school": "School",
    "college": "College",
    "university": "University",
    "library": "Library",
    "police": "Police Station",
    "fire_station": "Fire Station",
    "post_office": "Post Office",
    "community_centre": "Community Centre",
    "bus_station": "Bus Station",
    "parking": "Parking",
    "place_of_worship": "Place of Worship",
    "marketplace": "Market",
    "cinema": "Cinema",
    "theatre": "Theatre",
    "kindergarten": "Kindergarten",
    "childcare": "Childcare",
    "toilets": "Public Toilet",
}

SHOP_CATEGORIES = {
    "supermarket": "Supermarket",
    "convenience": "Convenience Store",
    "mall": "Shopping Mall",
    "department_store": "Department Store",
    "bakery": "Bakery",
    "butcher": "Butcher",
    "greengrocer": "Grocery Store",
    "clothes": "Clothing Store",
    "shoes": "Shoe Store",
    "electronics": "Electronics",
    "mobile_phone": "Mobile Store",
    "computer": "Computer Store",
    "books": "Bookstore",
    "chemist": "Pharmacy",
    "optician": "Optical Store",
    "jewelry": "Jewellery Store",
    "hardware": "Hardware Store",
    "furniture": "Furniture Store",
    "gift": "Gift Shop",
    "stationery": "Stationery Store",
    "beauty": "Beauty Store",
    "sports": "Sports Store",
    "pet": "Pet Store",
}

LEISURE_CATEGORIES = {
    "park": "Park",
    "fitness_centre": "Gym",
    "sports_centre": "Sports Centre",
    "playground": "Playground",
    "garden": "Garden",
}

TOURISM_CATEGORIES = {
    "hotel": "Hotel",
    "motel": "Motel",
    "guest_house": "Guest House",
    "hostel": "Hostel",
}

def build_query(lat, lon, radius):
    return f"""
[out:json][timeout:90];
(
  nwr(around:{radius},{lat},{lon})
    ["amenity"~"^(hospital|clinic|doctors|dentist|pharmacy|veterinary|restaurant|cafe|fast_food|food_court|bank|atm|fuel|school|college|university|library|police|fire_station|post_office|community_centre|bus_station|parking|place_of_worship|marketplace|cinema|theatre|kindergarten|childcare|toilets)$"];
  nwr(around:{radius},{lat},{lon})["shop"];
  nwr(around:{radius},{lat},{lon})
    ["leisure"~"^(park|fitness_centre|sports_centre|playground|garden)$"];
  nwr(around:{radius},{lat},{lon})
    ["tourism"~"^(hotel|motel|guest_house|hostel)$"];
);
out center tags;
"""

def fetch_osm(query):
    payload = urllib.parse.urlencode({"data": query}).encode("utf-8")
    headers = {
        "User-Agent": "GeoSearchEducationalProject/1.0",
        "Content-Type": "application/x-www-form-urlencoded",
    }

    last_error = None

    for endpoint in ENDPOINTS:
        for attempt in range(2):
            try:
                request = urllib.request.Request(
                    endpoint, data=payload, headers=headers, method="POST"
                )
                with urllib.request.urlopen(request, timeout=120) as response:
                    return json.loads(response.read().decode("utf-8"))
            except Exception as exc:
                last_error = exc
                print(f"Request failed ({endpoint}, attempt {attempt + 1}): {exc}")
                time.sleep(3)

    raise RuntimeError(f"All Overpass endpoints failed: {last_error}")

def get_coordinates(element):
    if "lat" in element and "lon" in element:
        return element["lat"], element["lon"]

    center = element.get("center", {})
    if "lat" in center and "lon" in center:
        return center["lat"], center["lon"]

    return None, None

def classify(tags):
    amenity = tags.get("amenity")
    if amenity in AMENITY_CATEGORIES:
        return AMENITY_CATEGORIES[amenity]

    shop = tags.get("shop")
    if shop:
        return SHOP_CATEGORIES.get(shop, "Shop")

    leisure = tags.get("leisure")
    if leisure in LEISURE_CATEGORIES:
        return LEISURE_CATEGORIES[leisure]

    tourism = tags.get("tourism")
    if tourism in TOURISM_CATEGORIES:
        return TOURISM_CATEGORIES[tourism]

    return "Other"

def make_description(tags):
    details = []

    for key, label in [
        ("brand", "Brand"),
        ("operator", "Operator"),
        ("cuisine", "Cuisine"),
        ("addr:suburb", "Area"),
        ("addr:street", "Street"),
        ("opening_hours", "Hours"),
    ]:
        value = tags.get(key)
        if value:
            details.append(f"{label}: {value}")

    return "; ".join(details) if details else "OpenStreetMap place listing"

def convert_elements(elements):
    rows = []
    seen = set()

    for element in elements:
        tags = element.get("tags", {})
        name = (tags.get("name") or "").strip()

        if not name:
            continue

        element_key = (element.get("type"), element.get("id"))
        if element_key in seen:
            continue
        seen.add(element_key)

        lat, lon = get_coordinates(element)
        if lat is None or lon is None:
            continue

        address = tags.get("addr:full", "")

        if not address:
            address_parts = [
                tags.get("addr:housenumber", ""),
                tags.get("addr:street", ""),
                tags.get("addr:suburb", ""),
                tags.get("addr:city", ""),
                tags.get("addr:postcode", ""),
            ]
            address = ", ".join(
                part for part in address_parts if part
            )

        rows.append({
            "name": name,
            "category": classify(tags),
            "description": make_description(tags),
            "latitude": lat,
            "longitude": lon,
            "cuisine": tags.get("cuisine", ""),
            "opening_hours": tags.get("opening_hours", ""),
            "phone": tags.get("contact:phone") or tags.get("phone", ""),
            "website": tags.get("contact:website") or tags.get("website", ""),
            "brand": tags.get("brand", ""),
            "operator": tags.get("operator", ""),
            "address": address,
        })

    rows.sort(key=lambda item: (item["category"].lower(), item["name"].lower()))

    for index, row in enumerate(rows, start=1):
        row["id"] = index

    return rows

def write_csv(rows, output_path):
    output_path.parent.mkdir(parents=True, exist_ok=True)
    temporary_path = output_path.with_suffix(".csv.tmp")

    fields = [
        "id", "name", "category", "description",
        "latitude", "longitude", "rating", "price",
        "cuisine", "opening_hours", "phone",
        "website", "brand", "operator", "address",
    ]

    with temporary_path.open("w", newline="", encoding="utf-8") as file:
        writer = csv.DictWriter(file, fieldnames=fields)
        writer.writeheader()

        for row in rows:
            writer.writerow({
                **row,
                "rating": "",
                "price": "",
                "cuisine": row["cuisine"],
                "opening_hours": row["opening_hours"],
                "phone": row["phone"],
                "website": row["website"],
                "brand": row["brand"],
                "operator": row["operator"],
                "address": row["address"],
            })

    temporary_path.replace(output_path)

def main():
    parser = argparse.ArgumentParser(
        description="Import nearby named places from OpenStreetMap."
    )
    parser.add_argument("--lat", type=float, default=DEFAULT_LAT)
    parser.add_argument("--lon", type=float, default=DEFAULT_LON)
    parser.add_argument("--radius", type=int, default=DEFAULT_RADIUS)
    parser.add_argument("--output", default="data/places.csv")
    args = parser.parse_args()

    if not -90 <= args.lat <= 90 or not -180 <= args.lon <= 180:
        parser.error("Invalid latitude or longitude.")
    if not 100 <= args.radius <= 50000:
        parser.error("Radius must be between 100 and 50000 metres.")

    output_path = Path(args.output)
    print(f"Centre: {args.lat}, {args.lon}")
    print(f"Radius: {args.radius} metres")
    print("Querying OpenStreetMap Overpass API...")

    result = fetch_osm(build_query(args.lat, args.lon, args.radius))
    rows = convert_elements(result.get("elements", []))

    if not rows:
        raise RuntimeError(
            "No named places were returned. Existing CSV was not changed."
        )

    if output_path.exists():
        backup_path = output_path.with_name("places.backup.csv")
        if backup_path.exists():
            backup_path = output_path.with_name("places.backup.previous.csv")
        backup_path.write_bytes(output_path.read_bytes())
        print(f"Existing CSV backed up to: {backup_path}")

    write_csv(rows, output_path)

    print()
    print(f"Imported places: {len(rows)}")
    print(f"Output file: {output_path}")
    print("Rating and price are blank because OSM does not reliably provide them.")
    print()
    print("Category counts:")

    counts = {}
    for row in rows:
        counts[row["category"]] = counts.get(row["category"], 0) + 1

    for category, count in sorted(counts.items()):
        print(f"  {category}: {count}")

    print()
    print("Data source: OpenStreetMap contributors")
    print("https://www.openstreetmap.org/copyright")

if __name__ == "__main__":
    main()
