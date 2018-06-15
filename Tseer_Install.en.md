# Installation Guide for TSeer 

## Dependent environments

Software |Software requirements
------|--------
linuxkernel version:      |	2.6.18 or later (Dependent OS)
gcc/g++ version:       	|   4.8.1 or later£¨Support c++11£©¡¢glibc-devel£¨C++ language framework and source compiler dependencies£©
bison version:      |	2.5 or later£¨C++ language framework and source compiler dependencies£©
flex version:       |	2.5 or later£¨C++ language framework and source compiler dependencies£©
cmake version£º       	|   2.8.8 or later£¨C++ language framework and source compiler dependencies£©
resin version£º       	|   4.0.49 or later£¨Web management system and source compiler dependencies£©
Java JDK version£º      |   web management system,£¨JDK 1.8 or later£©
Maven version£º		|   2.2.1 or later£¨web management system¡¢dependency of java framework£©
rapidjson version:      |   1.0.2£¨C++ language framework and source compiler dependencies£©

Hardware requirements: a machine running Linux.

## 1.Installation Method Selection

Tseer only provides source code compilation mode for installation, and it is highly customizable. Due to the convenience of one-click installation script, installation is not troublesome.

**Note:** wget and curl are required during installation.

#### Source installation

User need to pre-install python, g++, cmake, wget, curl, flex, bison, ensure that the machine network is normal, and clone the github repository.

## 2.Tars
    TSeer relies on [Tars](https://github.com/Tencent/Tars). One-click source installation script has included the automatic downloading and compilation of Tars, so user don¡¯t need to care about deployment of Tars.

**Note:** The source code compilation of Tars needs to install flex and bison in advance.

## 3.Source installation

#### Notice

- Before executing the installation script, be sure to `cd build/`
- Do not overlap the installation directory and the source directory.

## Download Source

Use git to download source: git clone https://github.com/Tencent/TSeer.git

1. Edit configuration file: vim build/tseer_deploy.ini, modify `install_type=source` to use source installation (For detailed usage, please refer to chapter 4 and perform the second step after editing.)
2. Perform the installation:`cd build; python tseer_deploy.py`

## 4.Edit Installation Configuration

The location of the installation configuration file: `tseer/build/tseer_deploy.ini`

### 4.1 Basic Configuration of TSeer

```ini
[tseer]
; install_type: bin, source; default: bin.
; binary install: place binary executable to your set base path
; source install: depend tars
install_type=bin
; storage: etcd, data storage.
storage=etcd
base_dir=/usr/local/
bind_ip=localhost
```

- `install_type`: `source` installation chooses will compile and install source code. `bin` installation directly uses the binary package already installed on git.
- `base_dir`: The service will be installed in the tseer directory under the `base_dir` directory.
- `bind_ip`: The listening address of TseerServer/TseerAgent. The default is the IP of this machine.

### 4.2 Configuration of TSeer

```ini
[tseer_server]
; agent registry and report heartbeat
regport=9902
; query service ip and port
queryport=9903
; admin port
apiport=9904
```

- `regport`: TSeerAgent registers and reports heartbeat to this port of TSeer Server and Web upload agent release packets to this port number  
- `queryport`: The port number is used for service routing information query
- `apiport`: This port number is used for WEB operation to route data and manage TSeer Agent.

### 4.3 Configuration of Storage Target


```ini
[etcd]
; host_list: localhost or like 10.0.0.1,192.168.0.10, 172.168.10.1
; localhost: start local 3 etcd instance
;host_list=localhost
client_port=2379
cluster_port=2380
base_dir=/data/test/etcd/
```

- `host_list`: To deploy etcd's host list, three etcd instances are started by default. Remote installation depends on the paramiko library, please install it yourself.  
- `client_port`: The port number is being accessed.
- `cluster_port`: The port number is used for internal communication in the etcd cluster

The default name of the etcd member is `tseer_etcd` plus sequence number, not configurable

## 5. Deploy the Web Platform 

The one-click installation script does not include the deployment of the web platform. Manual operation is required. The following is added here:

First download resin installation package from [here](http://caucho.com/download/resin-4.0.49.tar.gz).

```
cd build/
tar xvf resin-4.0.49.tar.gz
mv resin-4.0.49 resin
```

Modify the address of the query route
```
cd web 
vi src/main/resources/seer.conf
```
Change the address of locator to the Tseer Server's listening ip plus queryport number, the default port number is 9903

Modify the TSeer service address in the configuration. The modification is as follows:
```
cd web 
vi src/main/resources/system.properties
```

Modify the values of `seer.api.url` and `seer.agent.onekey.install.url` to the Tseer Server's listening ip you deploy and the `apiport` mentioned in the 4.2 section.

For example, if the Tseer Server's `bind_ip` is `localhost` and the `port` is `9904`, then modify this in the configuration file:
`seer.api.url=http://127.0.0.1:9904/v1/interface`

Now, start generating our war package, after executing the following command, you can see the `seer-1.0.0-SNAPSHOT.war` file in the `target` directory.

```
cd web 
mvn clean
mvn install
cp target/seer-1.0.0-SNAPSHOT.war ../build/resin/webapps/ 
```

Copy it to resin's `webapps` directory.

Set the running entrance of `resin`:

```
vi conf/resin.xml

Under this note, `webapps can be overridden/extended in the resin.xml`, adding
<web-app id="/" root-directory="webapps/seer-1.0.0-SNAPSHOT"/>
```

start resin: in the `resin` directory, execute:
```
./bin/resin.sh start
```

## 6. Start experiencing TSeer

### 6.1 Access Management Platform

Resin's default port is 8080, so enter http://127.0.0.1:8080 in your browser (IP is replaced with the real binding ip)

### 6.2 Add routing data

- In the upper right corner of the page, click on the 'Business Set List':

![endpoint](docs/images/group.png)

- After entering, click `Add Business Set`, enter the business set name:

![endpoint](docs/images/addgroup.png)

- Add module:

![endpoint](docs/images/addmodule.png)

- Add a specific route object:

The routing name of TSeer is four segments: business set, business name, service name, port name. When the business set is added, the business set name is confirmed, and the remaining three names need to be filled when adding the module. As shown in the figure, the business name is `app`, the service name is `server`, and the port name is `helloobj`.

![endpoint](docs/images/module.png)

### 6.3 Using TSeer API to get routing data

TSeer provides API for C++ and Java, placed in the API directory under the default installation path.

To learn how to use TSeer API, users can view corresponding API usage tutorials and examples.

[c++ api](docs/cplus-api-quickstart.md)

[java api](docs/seer-api-java-quickstart.md)

## 7. Install TSeer Agent
 
In the one-click install script,  the default installation starts the process of TSeer Agent. If user want to deploy TSer Agent on other machines, you need to install it yourself.

One-click install script helps users pack the agent packages with suffix tgz in `build` directory, and users need to upload them.

### 7.1 Upload the agent installation package by uploading the web page

- On the top right side of the web page, choose the `publish package management`, then select the upload TSeerAgent release package on the left side, as shown below.

![uploadagent](docs/images/uploadagent.png)

### 7.2 Through operating web page, the command to download agent is generated .

- In the upper right of the web page, user select `Agent Installation Tool`, fill in the target machine IP and web manager generate the command. User copy it and paste it on the target machine and run the command.

The generated command is similar to:

```
wget http://10.17.89.41:9904/installscript && python installscript --innerip=10.17.89.35
```

  `10.17.89.41` is the machine where you execute the one-click install script. `Innerip` is your target machine

  **Note** In general, the user also needs to specify the os version in the form of CentOs-7.2-64, otherwise the agent package cannot be downloaded. So the final command is similar to:

```
wget http://10.17.89.41:9904/installscript && python installscript --innerip=10.17.89.35 --os=CentOs-7.2-64
```

### 7.3 Log in to the target machine and execute the above command

   After the command is executed, the user uses the `ps` or `netstat` to check whether the agent process is started successfully.
