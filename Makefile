# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/07/13 20:25:55 by adaza-ru          #+#    #+#              #
#    Updated: 2026/08/05 17:52:28 by adaza-ru         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME        = codexion
CC          = cc
CFLAGS      = -Wall -Wextra -Werror -I include -g
RM          = rm -rf

SRCS_DIR    = src
OBJS_DIR    = obj

SRCS        = main.c\
			utils/time.c\
			utils/prints.c\
			utils/clean_up.c\
			init/arg_errors.c\
			init/init_env.c\
			init/start_simulation.c\
			core/watcher.c\
			core/coders.c\
			core/dongles.c\
			utils/queue.c

OBJS        = $(addprefix $(OBJS_DIR)/, $(SRCS:.c=.o))

STATE_MAN   = .mandatory
STATE_COL   = .color

LOG_ERR     = error_tests.log
LOG_HEL     = helgrind.log
LOG_TSAN    = tsan.log

.PHONY: all color clean fclean re recolor termtest logtest

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
	$(RM) $(LOG_ERR) $(LOG_HEL) $(LOG_TSAN)

fclean: clean
	$(RM) $(NAME)

re: fclean all

recolor: fclean color

termtest: $(NAME)
	@$(MAKE) recolor CFLAGS="$(CFLAGS)" > /dev/null
	@echo "Cleaning previous logs..."
	@$(RM) $(LOG_ERR) $(LOG_HEL) $(LOG_TSAN)
	
	@echo "========================================="
	@echo "      1/3. RUNNING PARSER/INIT TESTS     "
	@echo "========================================="
	@echo "Writing error cases output to '$(LOG_ERR)'..."
	
	@echo "\n[ERROR 1] Wrong number of arguments"
	-@./$(NAME) 4
	
	@echo "\n[ERROR 2] Negative arguments"
	-@./$(NAME) 4 400 -200 100 100 5 50 edf
	
	@echo "\n[ERROR 3] Argument with letters"
	-@./$(NAME) 4 400 200 100 100t 5 50 fifo
	
	@echo "\n[ERROR 4] Wrong scheduler"
	-@./$(NAME) 4 400 200 100 100 5 50 incorrecto
	
	@echo "\n[ERROR 5] Overflow INT_MAX"
	-@./$(NAME) 4 400 200 100 100 5 999999999999999 fifo

	@echo "\n[ERROR 6] Too many programmers"
	-@./$(NAME) 400 400 200 100 100 5 50 edf

	@echo "\n[ERROR 7] Fail creating a thread"
	-@bash -c "ulimit -u 20 && ./$(NAME) 250 800 200 200 200 3 100 edf"
	@echo "========================================="
	@echo "[OK] Error tests finished. Check '$(LOG_ERR)'\n"

	@echo "========================================="
	@echo "      2/3. RUNNING HELGRIND TESTS        "
	@echo "========================================="
	@echo "Executing 4 matrix tests with Helgrind (may take a few seconds)..."
	@echo "\n=== [TEST 1] FIFO + Burnout ==="
	-@valgrind --tool=helgrind --history-level=full ./$(NAME) 5 800 200 200 200 100 50 fifo
	@echo "\n=== [TEST 2] FIFO + Success ==="
	-@valgrind --tool=helgrind --history-level=full ./$(NAME) 4 1000 200 200 200 5 50 fifo
	@echo "\n=== [TEST 3] EDF + Burnout ==="
	-@valgrind --tool=helgrind --history-level=full ./$(NAME) 5 800 200 200 200 100 50 edf
	@echo "\n=== [TEST 4] EDF + Success ==="
	-@valgrind --tool=helgrind --history-level=full ./$(NAME) 4 1000 200 200 200 5 50 edf
	@echo ""

	@echo "========================================="
	@echo "   3/3. RUNNING THREADSANITIZER TESTS    "
	@echo "========================================="
	@echo "Recompiling binary with ThreadSanitizer flags..."
	@$(MAKE) recolor CFLAGS="$(CFLAGS) -fsanitize=thread" > /dev/null
	@echo "\n=== [TEST 1] FIFO + Burnout ==="
	-@./$(NAME) 5 800 200 200 200 100 50 fifo
	@echo "\n=== [TEST 2] FIFO + Success ==="
	-@./$(NAME) 4 1000 200 200 200 5 50 fifo
	@echo "\n=== [TEST 3] EDF + Burnout ==="
	-@./$(NAME) 5 800 200 200 200 100 50 edf
	@echo "\n=== [TEST 4] EDF + Success ==="
	-@./$(NAME) 4 1000 200 200 200 5 50 edf
	@echo "\nRestoring binary to standard compilation state..."
	@$(MAKE) recolor > /dev/null
	
	@echo "\n========================================"
	@echo "        ALL TESTS COMPLETED              "
	@echo "========================================="

logtest: $(NAME)
	@$(MAKE) re CFLAGS="$(CFLAGS)" > /dev/null
	@echo "Cleaning previous logs..."
	@$(RM) $(LOG_ERR) $(LOG_HEL) $(LOG_TSAN)
	
	@echo "========================================="
	@echo "      1/3. RUNNING PARSER/INIT TESTS     "
	@echo "========================================="
	@echo "Writing error cases output to '$(LOG_ERR)'..."
	
	@echo "\n[ERROR 1] Wrong number of arguments" > $(LOG_ERR) 2>&1
	-@./$(NAME) 4 >> $(LOG_ERR) 2>&1
	
	@echo "\n[ERROR 2] Negative arguments" >> $(LOG_ERR) 2>&1
	-@./$(NAME) 4 400 -200 100 100 5 50 edf >> $(LOG_ERR) 2>&1
	
	@echo "\n[ERROR 3] Argument with letters" >> $(LOG_ERR) 2>&1
	-@./$(NAME) 4 400 200 100 100t 5 50 fifo >> $(LOG_ERR) 2>&1
	
	@echo "\n[ERROR 4] Wrong scheduler" >> $(LOG_ERR) 2>&1
	-@./$(NAME) 4 400 200 100 100 5 50 incorrecto >> $(LOG_ERR) 2>&1
	
	@echo "\n[ERROR 5] Overflow INT_MAX" >> $(LOG_ERR) 2>&1
	-@./$(NAME) 4 400 200 100 100 5 999999999999999 fifo >> $(LOG_ERR) 2>&1

	@echo "\n[ERROR 6] Too many programmers" >> $(LOG_ERR) 2>&1
	-@./$(NAME) 400 400 200 100 100 5 50 edf >> $(LOG_ERR) 2>&1

	@echo "\n[ERROR 7] Fail creating a thread" >> $(LOG_ERR) 2>&1
	-@bash -c "ulimit -u 20 && ./$(NAME) 250 800 200 200 200 3 100 edf" >> $(LOG_ERR) 2>&1
	@echo "========================================="
	@echo "[OK] Error tests finished. Check '$(LOG_ERR)'\n"

	@echo "========================================="
	@echo "      2/3. RUNNING HELGRIND TESTS        "
	@echo "========================================="
	@echo "Executing 4 matrix tests with Helgrind (may take a few seconds)..."
	@echo "Logging to '$(LOG_HEL)'..."
	@echo "\n=== [TEST 1] FIFO + Burnout ===" > $(LOG_HEL)
	-@valgrind --tool=helgrind --history-level=full ./$(NAME) 5 800 200 200 200 100 50 fifo >> $(LOG_HEL) 2>&1
	@echo "\n=== [TEST 2] FIFO + Success ===" >> $(LOG_HEL)
	-@valgrind --tool=helgrind --history-level=full ./$(NAME) 4 1000 200 200 200 5 50 fifo >> $(LOG_HEL) 2>&1
	@echo "\n=== [TEST 3] EDF + Burnout ===" >> $(LOG_HEL)
	-@valgrind --tool=helgrind --history-level=full ./$(NAME) 5 800 200 200 200 100 50 edf >> $(LOG_HEL) 2>&1
	@echo "\n=== [TEST 4] EDF + Success ===" >> $(LOG_HEL)
	-@valgrind --tool=helgrind --history-level=full ./$(NAME) 4 1000 200 200 200 5 50 edf >> $(LOG_HEL) 2>&1
	@if grep -E "ERROR SUMMARY: [1-9]|possible data race|lock order violation" $(LOG_HEL) > /dev/null 2>&1; then \
		echo "\033[0;31m[HELGRIND ERROR DETECTED] Check $(LOG_HEL)\033[0m"; \
	else \
		echo "\033[0;32m[HELGRIND CLEAN]\033[0m"; \
	fi
	@echo ""

	@echo "========================================="
	@echo "   3/3. RUNNING THREADSANITIZER TESTS    "
	@echo "========================================="
	@echo "Recompiling binary with ThreadSanitizer flags..."
	@$(MAKE) re CFLAGS="$(CFLAGS) -fsanitize=thread" > /dev/null
	@echo "Executing 4 matrix tests with TSan, logging to '$(LOG_TSAN)'..."
	@echo "\n=== [TEST 1] FIFO + Burnout ===" > $(LOG_TSAN)
	-@./$(NAME) 5 800 200 200 200 100 50 fifo >> $(LOG_TSAN) 2>&1
	@echo "\n=== [TEST 2] FIFO + Success ===" >> $(LOG_TSAN)
	-@./$(NAME) 4 1000 200 200 200 5 50 fifo >> $(LOG_TSAN) 2>&1
	@echo "\n=== [TEST 3] EDF + Burnout ===" >> $(LOG_TSAN)
	-@./$(NAME) 5 800 200 200 200 100 50 edf >> $(LOG_TSAN) 2>&1
	@echo "\n=== [TEST 4] EDF + Success ===" >> $(LOG_TSAN)
	-@./$(NAME) 4 1000 200 200 200 5 50 edf >> $(LOG_TSAN) 2>&1
	@if grep -E "WARNING: ThreadSanitizer|data race|deadlock" $(LOG_TSAN) > /dev/null 2>&1; then \
		echo "\033[0;31m[TSAN ERROR DETECTED] Check $(LOG_TSAN)\033[0m"; \
	else \
		echo "\033[0;32m[TSAN CLEAN]\033[0m"; \
	fi
	@echo "\nRestoring binary to standard compilation state..."
	@$(MAKE) recolor > /dev/null
	
	@echo "\n========================================="
	@echo "        ALL TESTS COMPLETED              "
	@echo " Logs saved in:"
	@echo "   - Error Tests: $(LOG_ERR)"
	@echo "   - Helgrind:    $(LOG_HEL)"
	@echo "   - TSan:        $(LOG_TSAN)"
	@echo "========================================="