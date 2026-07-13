# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/07/13 20:25:55 by adaza-ru          #+#    #+#              #
#    Updated: 2026/07/13 21:05:11 by adaza-ru         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME        = codexion
CC          = cc
CFLAGS      = -Wall -Wextra -Werror -I include
RM          = rm -rf

SRCS_DIR    = src
OBJS_DIR    = obj

SRCS        = main.c

OBJS        = $(addprefix $(OBJS_DIR)/, $(SRCS:.c=.o))

STATE_MAN   = .mandatory
STATE_COL   = .color

.PHONY: all color clean fclean re

all: $(STATE_MAN)

color: $(STATE_COL)

$(STATE_MAN):
	@if [ -f $(STATE_COL) ]; then \
		$(RM) $(OBJS_DIR) $(STATE_COL); \
	fi
	@$(MAKE) $(NAME)
	@touch $(STATE_MAN)

$(STATE_COL):
	@if [ -f $(STATE_MAN) ]; then \
		$(RM) $(OBJS_DIR) $(STATE_MAN); \
	fi
	@$(MAKE) $(NAME) CFLAGS="$(CFLAGS) -D CONFIG_COLOR"
	@touch $(STATE_COL)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS_DIR) $(STATE_MAN) $(STATE_COL)

fclean: clean
	$(RM) $(NAME)

re: fclean all