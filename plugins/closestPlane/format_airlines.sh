# Go to https://en.wikipedia.org/wiki/List_of_airline_codes?oldformat=true
# Sort by IATA code
# Copy all the lines which have an IATA code into airlines_unformatted.dat

sed -e 's/\s\s\s/||/g' airlines_unformatted.dat | sed -e 's/\s\s/|/g' > airlines.dat
