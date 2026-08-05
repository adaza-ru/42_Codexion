/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dongles.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 02:30:11 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/08/05 17:29:31 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

static int	has_priority_conflict(t_env *env, t_coder *coder,
	int left, int right)
{
	int	i;
	int	ahead_id;
	int	ahead_left;
	int	ahead_right;

	i = 0;
	while (i < env->queue_size && env->queue[i] != coder->id)
	{
		ahead_id = env->queue[i];
		ahead_left = ahead_id;
		ahead_right = (ahead_id + 1) % env->num_coders;
		if (ahead_left == left || ahead_left == right
			|| ahead_right == left || ahead_right == right)
			return (1);
		i++;
	}
	return (0);
}

static int	can_take_dongles(t_env *env, t_coder *coder)
{
	int	left;
	int	right;

	left = coder->id;
	right = (coder->id + 1) % env->num_coders;
	if (left == right)
		return (0);
	if (env->heap_dongles[left] != 0 || env->heap_dongles[right] != 0)
		return (0);
	if (has_priority_conflict(env, coder, left, right))
		return (0);
	return (1);
}

static void	grab_dongles_state(t_env *env, t_coder *coder)
{
	int	left;
	int	right;

	left = coder->id;
	right = (coder->id + 1) % env->num_coders;
	env->heap_dongles[left] = coder->id + 1;
	env->heap_dongles[right] = coder->id + 1;
}

void	take_dongles(t_coder *coder)
{
	t_env	*env;

	env = coder->env;
	pthread_mutex_lock(&env->arbitrator_mutex);
	enqueue_coder(env, coder);
	while (!can_take_dongles(env, coder))
	{
		pthread_mutex_lock(&env->end_mutex);
		if (env->simulation_end)
		{
			pthread_mutex_unlock(&env->end_mutex);
			dequeue_coder(env, coder);
			pthread_mutex_unlock(&env->arbitrator_mutex);
			return ;
		}
		pthread_mutex_unlock(&env->end_mutex);
		pthread_cond_wait(&env->cond_coders[coder->id],
			&env->arbitrator_mutex);
	}
	dequeue_coder(env, coder);
	grab_dongles_state(env, coder);
	pthread_mutex_unlock(&env->arbitrator_mutex);
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
