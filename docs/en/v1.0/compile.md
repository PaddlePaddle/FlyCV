## FlyCV Compilation Instruction Manual

For easy compilation, we provide docker images for different platforms, you can also compile it on your local machine without docker.


Go to install docker（optional）: [Docker Installation](https://www.docker.com/)

### Contents

- <a href="#android">:bookmark: Compile Android Libraries</a>

- <a href="#armlinux">:bookmark: Compile Armlinux Libraries</a>

- <a href="#macos">:bookmark: Compile MacOS (x86 & arm) Libraries</a>

- <a href="#windows">:bookmark: Compile Windows Libraries</a>

- <a href="#linux">:bookmark: Compile Linux Libraries</a>

<br>

### <span id="android">For Android Libraries<span>

#### compile in docker container
  
```
docker pull flycv/x86_64:android_17c_23c
docker attach
source /etc/ndk/r17c.env
./scripts/armlinux/build.sh
```

#### compile on local machine
  
  
```
export ANDROID_NDK=/Users/taotianran/Library/Android/sdk/ndk/17.2.4988734/
./scripts/android/build.sh
```

<hr>

### <span id="armlinux">For Armlinux Libraries</span>
  
```
./scripts/armlinux/build.sh
```

<hr>


### <span id="macos">For Macos Libraries</span>
  
```
./scripts/macos/build_x86.sh
```

<hr>

### <span id="windows">For Windows Libraries</span>
  
```
./scripts/windows/build.sh
```

<hr>

### <span id="linux">For Linux Libraries</span>
  
```
./scripts/linux/build.sh
```
