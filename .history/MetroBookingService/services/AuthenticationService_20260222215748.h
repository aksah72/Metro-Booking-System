ingService$ curl -X POST http://localhoaman_sah@LAPTOP-R2NKHFGB:/mnt/d/Metro/MetroBookingService$ curl -X POST http://localhost:9090/booking \
-H "Content-Type: application/json" \
-H "Authorization: USER_1" \
-d '{"source":"A","destination":"E"}'
{"error":"Unauthorized"}aman_sah@LAPTOP-R2NKHFGB:/mnt/d/Metro/MetroBookingService$ curl -X POST http://localhost:9090/booking -H "Content-Type: application/json" -H "Authorization: aman@gmail.com_TOKEN_1" -d '{"source":"A","destination":"E"
{"bookingId":"B1","duration":13,"fare":25,"qr":"15371244144228519846","route":["A","B","C","D","E"],"segments":[{"line":"Y","stops":["A","B","C"]},{"line":"B","stops":["D","E"]}],"status":"PENDING","transfers":1}aman_sah@LAPTOP-R2NKHFGB:/mnaman_sah@LAPTOP-R2NKHFGB:/mnt/d/Metro/MetroBookingService$ curl -X POST http://localhost:9090/pay \
-H "Content-Type: application/json" \
-H "Authorization: USER_1" \
-d '{"bookingId":"1"}'
{"error":"Unauthorized"}aman_sah@LAPTOP-R2NKHFGB:/mnt/d/Metro/MetroBookingService$ curl -X POST http://localhost:9090/pay -H "Content-Type: application/jaman@gmail.com_TOKEN_1:amaman_sah@LAPTOP-R2NKHFGB:/mnt/d/Metro/MetroBookingService$ curl -X POST http://localhoaman_sah@LAPamanamanamanamanamanamanamanamanamanamanamanamanaman_sah@LAPTOP-R2NKHFGB:/mnt/d/Metro/MetroBookingService$ curl -X POST http://localhost:9090/pay -H "Content-Type: application/json" -H "Authorization: aman@gmail.com_TOKEN_1
" -d '{"bookingId":"1"}'
aman_sah@LAPTOP-R2NKHFGB:/mnt/d/Metro/MetroBookingService$ sqlite3 metro.db
SQLite version 3.45.1 2024-01-30 16:01:20
Enter ".help" for usage hints.
sqlite> SELECT * FROM users;
sqlite> SELECT * FROM bookings;
sqlite>