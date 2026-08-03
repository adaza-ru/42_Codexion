/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_env.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/13 20:25:49 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/08/02 02:03:31 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

static int	init_env_conders(t_env *env)
{
	int	i;

	i = 0;
	while (i < env->num_coders)
	{
		env->coders[i].id = i;
		env->coders[i].compiles_done = 0;
		env->coders[i].last_compile_start = 0;
		env->coders[i].env = env;
		if (pthread_mutex_init(&env->coders[i].state_mutex, NULL) != 0)
		{
			pthread_mutex_destroy(&env->arbitrator_mutex);
			pthread_mutex_destroy(&env->write_mutex);
			pthread_mutex_destroy(&env->end_mutex);
			while (--i >= 0)
				pthread_mutex_destroy(&env->coders[i].state_mutex);
			i = env->num_coders;
			while (--i >= 0)
				pthread_cond_destroy(&env->cond_coders[i]);
			return (15);
		}
		i++;
	}
	return (0);
}

static int	init_env_conds(t_env *env)
{
	int	i;

	i = 0;
	while (i < env->num_coders)
	{
		if (pthread_cond_init(&env->cond_coders[i], NULL) != 0)
		{
			pthread_mutex_destroy(&env->arbitrator_mutex);
			pthread_mutex_destroy(&env->write_mutex);
			pthread_mutex_destroy(&env->end_mutex);
			while (--i >= 0)
				pthread_cond_destroy(&env->cond_coders[i]);
			return (14);
		}
		env->heap_dongles[i] = 0;
		i++;
	}
	return (0);
}

static int	init_env_mutexes(t_env *env)
{
	if (pthread_mutex_init(&env->arbitrator_mutex, NULL) != 0)
		return (11);
	if (pthread_mutex_init(&env->write_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&env->arbitrator_mutex);
		return (12);
	}
	if (pthread_mutex_init(&env->end_mutex, NULL) != 0)
	{
		pthread_mutex_destroy(&env->arbitrator_mutex);
		pthread_mutex_destroy(&env->write_mutex);
		return (13);
	}
	return (0);
}

static void	init_env_data(t_env *env, char **argv)
{
	env->num_coders = atoi(argv[1]);
	env->time_to_burnout = atoi(argv[2]) * 1000;
	env->time_to_compile = atoi(argv[3]) * 1000;
	env->time_to_debug = atoi(argv[4]) * 1000;
	env->time_to_refactor = atoi(argv[5]) * 1000;
	env->num_compiles_required = atoi(argv[6]);
	env->dongle_cooldown = atoi(argv[7]) * 1000;
	if (strcmp(argv[8], "edf") == 0)
		env->scheduler = 1;
	else
		env->scheduler = 0;
	env->queue_size = 0;
	env->simulation_end = 0;
}

int	init_env(t_env *env, char **argv)
{
	int	err_mark;

	init_env_data(env, argv);
	err_mark = init_env_mutexes(env);
	if (err_mark)
		return (err_mark);
	err_mark = init_env_conds(env);
	if (err_mark)
		return (err_mark);
	err_mark = init_env_conders(env);
	return (err_mark);
}
