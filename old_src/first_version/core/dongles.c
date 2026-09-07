/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 02:30:11 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/08/12 19:06:17 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

static int	is_doomed(t_coder *coder, int ahead)
{
	t_env	*env;
	size_t	time_lived;
	size_t	time_left;
	size_t	time_needed;
	size_t	margin;

	env = coder->env;
	time_lived = get_current_time() - coder->last_compile_start;
	if (time_lived >= env->time_to_burnout)
		return (1);
	time_left = env->time_to_burnout - time_lived;
	margin = env->time_to_burnout / 10;
	if (margin > 15)
		margin = 15;
	if (time_left <= margin)
		return (1);
	time_needed = (ahead + 1) * (env->time_to_compile + env->dongle_cooldown);
	if (time_needed >= (time_left - margin))
		return (1);
	return (0);
}

static int	can_take_dongles(t_coder *coder)
{
	t_env	*env;
	int		left;
	int		right;
	int		i;
	int		other_id;

	env = coder->env;
	left = coder->id;
	right = (coder->id + 1) % env->num_coders;
	if (env->heap_dongles[left] != 0 || env->heap_dongles[right] != 0)
		return (0);
	i = 0;
	while (i < env->queue_size)
	{
		other_id = env->queue[i];
		if (other_id == coder->id)
			break ;
		if (other_id == left || other_id == right
			|| (other_id + 1) % env->num_coders == left
			|| (other_id + 1) % env->num_coders == right)
			if (env->scheduler == 0 || !is_doomed(coder, i))
				return (0);
		i++;
	}
	return (1);
}

int	take_dongles(t_coder *coder)
{
	t_env	*env;
	int		left;
	int		right;

	env = coder->env;
	if (check_simulation_end(env))
		return (0);
	left = coder->id;
	right = (coder->id + 1) % env->num_coders;
	pthread_mutex_lock(&env->arbitrator_mutex);
	enqueue_coder(env, coder);
	while (!can_take_dongles(coder) && !check_simulation_end(env))
		pthread_cond_wait(&env->cond_coders[coder->id], &env->arbitrator_mutex);
	if (!check_simulation_end(env))
	{
		env->heap_dongles[left] = 1;
		env->heap_dongles[right] = 1;
		dequeue_coder(env, coder);
	}
	else
		dequeue_coder(env, coder);
	pthread_mutex_unlock(&env->arbitrator_mutex);
	return (1);
}

void	release_dongles(t_coder *coder)
{
	t_env	*env;
	int		left_neighbor;
	int		right_neighbor;

	env = coder->env;
	pthread_mutex_lock(&env->arbitrator_mutex);
	env->heap_dongles[coder->id] = 0;
	env->heap_dongles[(coder->id + 1) % env->num_coders] = 0;
	left_neighbor = (coder->id - 1 + env->num_coders) % env->num_coders;
	right_neighbor = (coder->id + 1) % env->num_coders;
	pthread_cond_broadcast(&env->cond_coders[left_neighbor]);
	pthread_cond_broadcast(&env->cond_coders[right_neighbor]);
	pthread_mutex_unlock(&env->arbitrator_mutex);
}
