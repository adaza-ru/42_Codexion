/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   start_simulation.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 01:42:39 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/09/09 02:02:32 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

static int	handle_creation_error(t_env *env, int created_count)
{
	int	i;

	pthread_mutex_lock(&env->end_mutex);
	env->simulation_end = 1;
	pthread_mutex_unlock(&env->end_mutex);
	pthread_mutex_lock(&env->arbitrator_mutex);
	i = 0;
	while (i < env->num_coders)
	{
		pthread_cond_broadcast(&env->cond_coders[i]);
		i++;
	}
	pthread_mutex_unlock(&env->arbitrator_mutex);
	pthread_join(env->watcher_thread, NULL);
	while (--created_count >= 0)
		pthread_join(env->coder_threads[created_count], NULL);
	return (17);
}

static void	join_all_threads(t_env *env)
{
	int	i;

	i = 0;
	pthread_join(env->watcher_thread, NULL);
	while (i < env->num_coders)
	{
		pthread_join(env->coder_threads[i], NULL);
		i++;
	}
}

int	start_simulation(t_env *env)
{
	int	i;

	i = 0;
	env->start_time = get_current_time();
	while (i < env->num_coders)
	{
		pthread_mutex_lock(&env->coders[i].state_mutex);
		env->coders[i].last_compile_start = env->start_time;
		pthread_mutex_unlock(&env->coders[i].state_mutex);
		i++;
	}
	if (pthread_create(&env->watcher_thread, NULL, watcher_routine, env) != 0)
		return (16);
	i = 0;
	while (i < env->num_coders)
	{
		if (pthread_create(&env->coder_threads[i], NULL,
				coder_routine, &env->coders[i]) != 0)
			return (handle_creation_error(env, i));
		i++;
	}
	join_all_threads(env);
	return (0);
}
