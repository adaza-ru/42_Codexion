/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   h_codexion.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 23:00:46 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/08/05 14:41:46 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef H_CODEXION_H
# define H_CODEXION_H

# include <pthread.h>
# include <sys/time.h>
# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>

# ifdef CONFIG_COLOR
#  define RESET          "\033[0m"
#  define TIMESTAMP_CLR  "\033[1;90m"
#  define ID_CLR         "\033[1;36m"
#  define DONGLE_CLR     "\033[0;33m"
#  define COMPILE_CLR    "\033[0;32m"
#  define DEBUG_CLR      "\033[0;34m"
#  define REFACTOR_CLR   "\033[0;35m"
#  define DEATH_CLR      "\033[1;31m"
#  define CLR_SUCCESS    "\033[1;32m"
# else
#  define RESET          ""
#  define TIMESTAMP_CLR  ""
#  define ID_CLR         ""
#  define DONGLE_CLR     ""
#  define COMPILE_CLR    ""
#  define DEBUG_CLR      ""
#  define REFACTOR_CLR   ""
#  define DEATH_CLR      ""
#  define CLR_SUCCESS    ""
# endif

typedef struct s_env	t_env;

typedef struct s_coder
{
	int				id;
	int				compiles_done;
	size_t			last_compile_start;
	pthread_mutex_t	state_mutex;
	t_env			*env;
}	t_coder;

typedef struct s_env
{
	int				num_coders;
	size_t			time_to_burnout;
	size_t			time_to_compile;
	size_t			time_to_debug;
	size_t			time_to_refactor;
	int				num_compiles_required;
	size_t			dongle_cooldown;
	int				scheduler;
	int				heap_dongles[250];
	int				queue[250];
	int				queue_size;
	int				simulation_end;
	size_t			start_time;
	pthread_mutex_t	arbitrator_mutex;
	pthread_mutex_t	write_mutex;
	pthread_cond_t	cond_coders[250];
	pthread_mutex_t	end_mutex;
	t_coder			coders[250];
	pthread_t		coder_threads[250];
	pthread_t		watcher_thread;
}	t_env;

int		check_arg_errors(int argc, char **argv);
void	print_error(int n);
int		init_env(t_env *env, char **argv);
void	clean_up_everything(t_env *env);
int		start_simulation(t_env *env);
void	codex_usleep(size_t milliseconds, t_env *env);
void	*watcher_routine(void *arg);
size_t	get_current_time(void);
void	print_status(t_coder *coder, char *color, char *status);
void	*coder_routine(void *arg);
void	take_dongles(t_coder *coder);
void	release_dongles(t_coder *coder);
void	enqueue_coder(t_env *env, t_coder *coder);
void	dequeue_coder(t_env *env, t_coder *coder);

#endif