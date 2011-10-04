(
	cd java
	if [ -z $http_proxy ]
	then
		false
	else
		http_proxy_host=$(echo $http_proxy | sed 's@http://@@')  # Removes the "http://" prefix
		http_proxy_port=-J-Dhttp.proxyPort=$(echo $http_proxy_host | sed 's@.*:@@') # Extracts the port
		http_proxy_host=-J-Dhttp.proxyHost=$(echo $http_proxy_host | sed 's@:.*@@') # Extracts the host
		echo Using proxy settings:
		echo $http_proxy_host
		echo $http_proxy_port
	fi
	javadoc $http_proxy_host $http_proxy_port -link http://download.oracle.com/javase/6/docs/api/ -public -d docs mtree mtree.utils
	unset http_proxy_host http_proxy_port
)
