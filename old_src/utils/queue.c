/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   queue.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 13:57:05 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/08/23 20:35:59 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

static size_t	get_coder_deadline(t_env *env, int coder_id)
{
	size_t	last_start;

	pthread_mutex_lock(&env->coders[coder_id].state_mutex);
	last_start = env->coders[coder_id].last_compile_start;
	pthread_mutex_unlock(&env->coders[coder_id].state_mutex);
	return (last_start + env->time_to_burnout);
}

static void	swap_nodes(int *a, int *b)
{
	int	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

static int	has_higher_priority(t_env *env, int coder_a, int coder_b)
{
	size_t	deadline_a;
	size_t	deadline_b;

	if (env->scheduler == 0)
		return (coder_a < coder_b);
	deadline_a = get_coder_deadline(env, coder_a);
	deadline_b = get_coder_deadline(env, coder_b);
	if (deadline_a == deadline_b)
		return (coder_a < coder_b);
	return (deadline_a < deadline_b);
}

static void	sift_up(t_env *env, int idx)
{
	int	parent;

	while (idx > 0)
	{
		parent = (idx - 1) / 2;
		if (has_higher_priority(env, env->queue[idx], env->queue[parent]))
		{
			swap_nodes(&env->queue[idx], &env->queue[parent]);
			idx = parent;
		}
		else
			break ;
	}
}

static void	sift_down(t_env *env, int idx)
{
	int	smallest;
	int	left;
	int	right;

	while (1)
	{
		smallest = idx;
		left = 2 * idx + 1;
		right = 2 * idx + 2;
		if (left < env->queue_size
			&& has_higher_priority(env, env->queue[left], env->queue[smallest]))
			smallest = left;
		if (right < env->queue_size
			&& has_higher_priority(
				env, env->queue[right], env->queue[smallest]))
			smallest = right;
		if (smallest != idx)
		{
			swap_nodes(&env->queue[idx], &env->queue[smallest]);
			idx = smallest;
		}
		else
			break ;
	}
}

void	enqueue_coder(t_env *env, t_coder *coder)
{
	env->queue[env->queue_size] = coder->id;
	sift_up(env, env->queue_size);
	env->queue_size++;
}

void	dequeue_coder(t_env *env, t_coder *coder)
{
	int	i;

	i = 0;
	while (i < env->queue_size && env->queue[i] != coder->id)
		i++;
	if (i < env->queue_size)
	{
		env->queue[i] = env->queue[env->queue_size - 1];
		env->queue_size--;
		if (i < env->queue_size)
		{
			sift_down(env, i);
			sift_up(env, i);
		}
	}
}
