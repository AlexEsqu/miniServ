FROM alpine:3.20

# Install PHP-FPM and required extensions
RUN apk update && apk add --no-cache \
    php \
    php-fpm \
    php-mysqli \
    php-json \
    php-curl \
    php-dom \
    php-exif \
    php-fileinfo \
    php-mbstring \
    php-openssl \
    php-xml \
    php-zip \
    php-redis \
    php-ftp \
    php-ctype \
    php-session \
    php-tokenizer \
    php-imagick \
    php-gd \
    php-iconv \
    php-phar \
    php-sodium \
    mysql-client \
    curl \
    && rm -rf /var/cache/apk/*

# Create necessary directories
RUN mkdir -p /var/www/html /var/log/php /run/php

# Ensure container is listening for connection on the correct port
RUN sed -i 's/listen = 127.0.0.1:9000/listen = 0.0.0.0:9000/g'  /etc/php83/php-fpm.d/www.conf

# Expose PHP-FPM port
EXPOSE 9000

# start  PHP-FPM
CMD ["php-fpm83", "--nodaemonize"]
