# **************************************************************************** #
#		General variables													   #
# **************************************************************************** #

NAME				=	webserv

SRC_DIR				=	src
OBJ_DIR				=	obj
INC_DIR				=	inc
WEB_DIR				=	pages

#----- SOURCE FILE FOLDERS ----------------------------------------------------#

DIR_ERR				=	error
DIR_ENCOD			=	response
DIR_DECOD			=	request
DIR_PARS			=	parsing
DIR_EXEC			=	execution
DIR_FILE			=	fileserv
DIR_CLAS			=	classes
DIR_POLL			=	polling

#----- SOURCE FILES -----------------------------------------------------------#

FUNC_ERR			=	error.cpp
FUNC_ENCOD			=	response.cpp Response.cpp
FUNC_DECOD			=	Request.cpp
FUNC_PARS			=	parsing.cpp
FUNC_EXEC			=	execution.cpp
FUNC_FILE			=	fileserv.cpp
FUNC_CLAS			=	Status.cpp
FUNC_POLL			=	Sockette.cpp SocketteAnswer.cpp SocketteListen.cpp

FUNC				=	$(addprefix $(DIR_ERR)/, $(FUNC_ERR)) \
						$(addprefix $(DIR_ENCOD)/, $(FUNC_ENCOD)) \
						$(addprefix $(DIR_DECOD)/, $(FUNC_DECOD)) \
						$(addprefix $(DIR_PARS)/, $(FUNC_PARS)) \
						$(addprefix $(DIR_EXEC)/, $(FUNC_EXEC)) \
						$(addprefix $(DIR_FILE)/, $(FUNC_FILE)) \
						$(addprefix $(DIR_CLAS)/, $(FUNC_CLAS)) \
						$(addprefix $(DIR_POLL)/, $(FUNC_POLL)) \
						main.cpp

HEAD				=	server.hpp

SRC					=	$(addprefix $(SRC_DIR)/, $(FUNC))

HEADER				=	$(addprefix $(INC_DIR)/, $(HEAD))

#----- COMPILATION VARIABLES --------------------------------------------------#

INC					=	-I$(INC_DIR)

OBJ_DIRS			= 	$(OBJ_DIR) \
						$(OBJ_DIR)/$(DIR_ENCOD) \
						$(OBJ_DIR)/$(DIR_DECOD) \
						$(OBJ_DIR)/$(DIR_ERR) \
						$(OBJ_DIR)/$(DIR_EXEC) \
						$(OBJ_DIR)/$(DIR_PARS) \
						$(OBJ_DIR)/$(DIR_CLAS) \
						$(OBJ_DIR)/$(DIR_FILE) \
						$(OBJ_DIR)/$(DIR_POLL)

OBJ					=	$(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

DEP					=	$(OBJ_DIRS) $(HEADER)

CC					=	c++

FLAGS				=	-Wall -Wextra -Werror -std=c++98

# **************************************************************************** #
#		Testing variables													   #
# **************************************************************************** #

TEST_DIR			=	tests

#------- C Client -------------------------------------------------------------#

CLI_DIR				=	$(TEST_DIR)/clients
CCLIENT				=	$(CLI_DIR)/client.cpp
CCLIENT_NAME		=	client

#------ Go Client -------------------------------------------------------------#



#------ HTTP Requests ---------------------------------------------------------#

TEST_REQ_DIR		=	$(TEST_DIR)/requests
HTTP_TEST			=	$(TEST_REQ_DIR)/correct.http

# **************************************************************************** #
#		Server																   #
# **************************************************************************** #

all:				$(NAME)

$(NAME):			$(OBJ_DIRS) $(OBJ)
					$(CC) $(FLAGS) $(INC) -o $(NAME) $(OBJ)

$(OBJ_DIR)/%.o:		$(SRC_DIR)/%.cpp $(HEADER)
					$(CC) $(FLAGS) $(INC) -c $< -o $@

$(OBJ_DIRS):
					mkdir -p $(OBJ_DIRS)

# **************************************************************************** #
#		Client																   #
# **************************************************************************** #

$(CCLIENT_NAME):
					$(CC) $(FLAGS) -o $(CCLIENT_NAME) $(CCLIENT)


# **************************************************************************** #
#		Debug																   #
# **************************************************************************** #

debug:				$(OBJ_DIRS) $(OBJ)
					@echo "Compiling with debug flag"
					$(CC) $(FLAGS) -g $(INC) -o $(NAME) $(SRC)

verbose:			$(OBJ_DIRS) $(OBJ)
					@echo "Compiling with additional logging info"
					$(CC) $(FLAGS) -D DEBUG -g $(INC) -o $(NAME) $(SRC)

# **************************************************************************** #
#		Clean up															   #
# **************************************************************************** #

clean:
					rm -rf $(OBJ_DIR)

fclean:
					rm -rf $(OBJ_DIR)
					rm -rf $(NAME)

re:					fclean all

.PHONY:				all clean fclean re debug verbose $(CCLIENT_NAME)
