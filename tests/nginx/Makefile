COMPOSE_FILE = docker-compose.yml
PROJECT_NAME = inception
DOMAIN = mkling.42.fr
DOCKER_COMPOSE = docker compose -f $(COMPOSE_FILE) -p $(PROJECT_NAME)

all: build up

build:
	$(DOCKER_COMPOSE) build

up:
	$(DOCKER_COMPOSE) up -d

down:
	$(DOCKER_COMPOSE) down

restart: down up

logs:
	$(DOCKER_COMPOSE) logs -f

status:
	$(DOCKER_COMPOSE) ps

clean:
	$(DOCKER_COMPOSE) down --remove-orphans
	docker system prune -f

fclean:
	$(DOCKER_COMPOSE) down --volumes --remove-orphans
	docker system prune -af
	docker volume prune -f

re: fclean build up


.PHONY: all build up down restart logs status clean fclean re
