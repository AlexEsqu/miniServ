FROM alpine:3.20

# install nginx, and openssl for SSL certificates
RUN apk update && apk add \
    nginx \
	curl \
    openssl

# copy nginx configuration in nginx
COPY conf/nginx.conf /etc/nginx/nginx.conf

# expose HTTPS port
EXPOSE 8080

# start nginx with a weirdass command ending in a semicolon cuz php
CMD ["nginx", "-g", "daemon off;"]
