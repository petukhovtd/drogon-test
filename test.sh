#/bin/bash

# curl 'http://localhost:3000/'

# echo

# curl -d '{"name":"Xenia"}' -H "Content-Type: application/json" -X POST http://localhost:3000/

# echo

for (( i=1; i <= 1000; i++ ))
do
    echo $i
    curl -d '{"login":"Xenia'$i'", "password":"123"}' -H "Content-Type: application/json" -X POST -i http://localhost:3000/api/v1/user/create
    echo
done

exit 0

curl -d '{"login":"Xenia2", "password":"123"}' -H "Content-Type: application/json" -X POST -i http://localhost:3000/api/v1/user/create

echo