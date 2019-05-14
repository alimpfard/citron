import json
with open("generated (1).json") as js:
    ldd = json.load(js)
(json.dumps(ldd)) #Python is not lazy, so this is fine

