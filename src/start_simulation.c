/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   start_simulation.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 01:42:39 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/08/04 00:02:13 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

static void	*coder_routine(void *arg)
{
	(void)arg;
	return (NULL);
}

static int	handle_creation_error(t_env *env, int created_count)
{
	pthread_mutex_lock(&env->end_mutex);
	env->simulation_end = 1;
	pthread_mutex_unlock(&env->end_mutex);
	pthread_join(env->watcher_thread, NULL);
	while (--created_count >= 0)
		pthread_join(env->coder_threads[created_count], NULL);
	return (17);
}

static void	join_all_threads(t_env *env)
{
	int	i;

	pthread_join(env->watcher_thread, NULL);
	i = 0;
	while (i < env->num_coders)
	{
		pthread_join(env->coder_threads[i], NULL);
		i++;
	}
}

int	start_simulation(t_env *env)
{
	int	i;

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
