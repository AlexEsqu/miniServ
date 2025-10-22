# **************************************************************************** #
#		General variables													   #
# **************************************************************************** #

NAME				=	webserv

SRC_DIR				=	src
OBJ_DIR				=	obj
INC_DIR				=	inc
WEB_DIR				=	pages

#----- SOURCE FILE FOLDERS ----------------------------------------------------#

DIR_FILL			=	filling
DIR_ENCOD			=	response
DIR_DECOD			=	request
DIR_PARS			=	parsing
DIR_EXEC			=	execution
DIR_POLL			=	polling
DIR_CONF			=	config
DIR_SIG				=	signal
DIR_BUFF			=	buffer

#----- SOURCE FILES -----------------------------------------------------------#

FUNC_FILL			=	ContentFetcher.cpp ContentPOSTer.cpp \
						ContentDELETEr.cpp ContentGETter.cpp
FUNC_ENCOD			=	Response.cpp Status.cpp
FUNC_DECOD			=	Request.cpp
FUNC_PARS			=	parsing.cpp ConfigParser.cpp
FUNC_EXEC			=	Executor.cpp PHPExecutor.cpp PythonExecutor.cpp
FUNC_CONF			=	ServerConf.cpp Route.cpp
FUNC_POLL			=	Sockette.cpp ClientSocket.cpp ServerSocket.cpp Poller.cpp
FUNC_SIG			=	signal.cpp
FUNC_BUFF			=	Buffer.cpp FileHandler.cpp

FUNC				=	$(addprefix $(DIR_FILL)/, $(FUNC_FILL)) \
						$(addprefix $(DIR_ENCOD)/, $(FUNC_ENCOD)) \
						$(addprefix $(DIR_DECOD)/, $(FUNC_DECOD)) \
						$(addprefix $(DIR_PARS)/, $(FUNC_PARS)) \
						$(addprefix $(DIR_EXEC)/, $(FUNC_EXEC)) \
						$(addprefix $(DIR_POLL)/, $(FUNC_POLL)) \
						$(addprefix $(DIR_SIG)/, $(FUNC_SIG)) \
						$(addprefix $(DIR_CONF)/, $(FUNC_CONF)) \
						$(addprefix $(DIR_BUFF)/, $(FUNC_BUFF)) \
						main.cpp

DIRS				=	$(DIR_FILL) $(DIR_ENCOD) $(DIR_DECOD) $(DIR_PARS) \
						$(DIR_EXEC) $(DIR_POLL) $(DIR_SIG) $(DIR_CONF) $(DIR_BUFF)

SRC					=	$(addprefix $(SRC_DIR)/, $(FUNC))

#----- HEADER FILES -----------------------------------------------------------#

HEAD_DIR			=	$(addprefix $(INC_DIR)/, $(DIRS))

#----- COMPILATION VARIABLES --------------------------------------------------#

INC					=	$(addprefix -I, $(HEAD_DIR)) -I$(INC_DIR)

OBJ_DIRS			= 	$(OBJ_DIR) \
						$(OBJ_DIR)/$(DIR_ENCOD) \
						$(OBJ_DIR)/$(DIR_DECOD) \
						$(OBJ_DIR)/$(DIR_FILL) \
						$(OBJ_DIR)/$(DIR_EXEC) \
						$(OBJ_DIR)/$(DIR_PARS) \
						$(OBJ_DIR)/$(DIR_POLL) \
						$(OBJ_DIR)/$(DIR_SIG) \
						$(OBJ_DIR)/$(DIR_HAND) \
						$(OBJ_DIR)/$(DIR_CONF) \
						$(OBJ_DIR)/$(DIR_BUFF)

OBJ					=	$(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

DEP					=	$(OBJ_DIRS) $(HEADER)

CC					=	c++

FLAGS				=	-Wall -Wextra -Werror -std=c++98

#----- RUNTIME VARIABLES ------------------------------------------------------#

TMP_DIR				=	tmp/

# **************************************************************************** #
#		Testing variables													   #
# **************************************************************************** #

TEST_DIR			=	tests
DEBUG_FLAG			=	-g #-fno-limit-debug-info # for full string view on clang

#------- NGINX Docker ---------------------------------------------------------#

NGINX_DOCK			=	$(TEST_DIR)/nginx

#------- C Client -------------------------------------------------------------#

CLI_DIR				=	$(TEST_DIR)/clients
CCLIENT				=	$(CLI_DIR)/client.cpp
CCLIENT_NAME		=	client

#------ Go Client -------------------------------------------------------------#



#------ HTTP Requests ---------------------------------------------------------#

TEST_REQ_DIR		=	$(TEST_DIR)/requests
HTTP_TEST			=	$(TEST_REQ_DIR)/correct.http

#------ Unit Test -------------------------------------------------------------#

UNIT_TEST_DIR		=	$(TEST_DIR)/unittest
UNIT_TEST_SRC		=	$(UNIT_TEST_DIR)/*.cpp
UNIT_TEST_FRAME		=	$(UNIT_TEST_DIR)/doctest/doctest/
SRC_NO_MAIN			=	$(filter-out $(SRC_DIR)/main.cpp, $(SRC))
UNIT_TEST_BIN		=	utest_$(NAME)

#------ Valgrind --------------------------------------------------------------#

V_FLAG				= valgrind --leak-check=full --show-leak-kinds=all \
						--track-origins=yes --track-fds=yes \
						--trace-children=yes

# **************************************************************************** #
#		Server																   #
# **************************************************************************** #

all:				$(NAME)

start:
					make re
					./$(NAME)

$(NAME):			$(OBJ_DIRS) $(OBJ) $(TMP_DIR)
					$(CC) $(FLAGS) $(INC) -o $(NAME) $(OBJ)

$(OBJ_DIR)/%.o:		$(SRC_DIR)/%.cpp $(HEADER)
					$(CC) $(FLAGS) $(INC) -c $< -o $@

$(OBJ_DIRS):
					mkdir -p $(OBJ_DIRS)

$(TMP_DIR):
					mkdir -p $(TMP_DIR)

# **************************************************************************** #
#		Testing																   #
# **************************************************************************** #

nginx:
					make -C $(NGINX_DOCK)
					@echo "Connect on http://localhost:8080"

$(CCLIENT_NAME):
					$(CC) $(FLAGS) -o $(CCLIENT_NAME) $(CCLIENT)

test:				$(TMP_DIR)
					$(CC) -g -I$(UNIT_TEST_FRAME) $(INC) -o $(UNIT_TEST_BIN) $(UNIT_TEST_SRC) $(SRC_NO_MAIN)
					./utest_webserv -ni -nv

# **************************************************************************** #
#		Debug																   #
# **************************************************************************** #

debug:				$(TMP_DIR)
					@echo "Compiling with debug flag"
					$(CC) $(FLAGS) $(DEBUG_FLAG)  $(INC) -o $(NAME) $(SRC)

verbose:			$(TMP_DIR)
					@echo "Compiling with additional logging info"
					$(CC) $(FLAGS) -D DEBUG $(DEBUG_FLAG) $(INC) -o $(NAME) $(SRC)

valgrind:
					make debug
					$(V_FLAG) ./webserv confs/basic.conf

# **************************************************************************** #
#		Clean up															   #
# **************************************************************************** #

clean:
					rm -rf $(OBJ_DIR)
					rm -rf $(UNIT_TEST_BIN)
					make -C $(NGINX_DOCK) clean

fclean:
					make clean
					rm -rf $(NAME)

re:					fclean all

.PHONY:				all clean fclean re debug verbose $(CCLIENT_NAME)
