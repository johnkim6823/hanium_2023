# Web-UI    

### 0. WEBUI테스트 방법  Tageun's TestBed 
```   
Linux-Raspberry PI
terminal 1  
$ **Connect PI-> ip:125.191.55.184 port:8001
$ **id:pi pw:pi  
$ cd hanium_2022 
$ cd Server  
$ make clean && make  
$ ./serverTest

terminal 2  
$ cd hanium_2022  
$ cd Logger  
$ make clean && make    
$ ./Logger

terminal 3  
$ cd Web-UI  
$ python3 web.py
$ click->Running on http://0.0.0.0:8080
```



