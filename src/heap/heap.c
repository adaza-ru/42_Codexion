/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 02:30:11 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/09/06 02:41:27 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

static void	sift_up(t_env *env, int idx)
{
	int	parent;

	while (idx > 0)
	{
		parent = (idx - 1) / 2;
		if (has_higher_priority(env, env->heap[idx], env->heap[parent]))
		{
			swap_nodes(&env->heap[idx], &env->heap[parent]);
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
		if (left < env->heap_size
			&& has_higher_priority(env, env->heap[left], env->heap[smallest]))
			smallest = left;
		if (right < env->heap_size
			&& has_higher_priority(env, env->heap[right], env->heap[smallest]))
			smallest = right;
		if (smallest == idx)
			break ;
		swap_nodes(&env->heap[idx], &env->heap[smallest]);
		idx = smallest;
	}
}

void	dequeue_coder(t_env *env, t_coder *coder)
{
	int	i;

	i = 0;
	while (i < env->heap_size && env->heap[i] != coder->id)
		i++;
	if (i < env->heap_size)
	{
		env->heap[i] = env->heap[env->heap_size - 1];
		env->heap_size--;
		if (i < env->heap_size)
		{
			sift_down(env, i);
			sift_up(env, i);
		}
	}
}

void	enqueue_coder(t_env *env, t_coder *coder)
{
	env->arrival_seq[coder->id] = env->next_seq;
	env->next_seq++;
	env->heap[env->heap_size] = coder -> id;
	sift_up(env, env->heap_size);
	env->heap_size++;
}
