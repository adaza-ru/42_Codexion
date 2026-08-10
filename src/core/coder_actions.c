/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_actions.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 21:38:06 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/08/10 03:09:34 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

static int	are_all_compiles_done(t_env *env)
{
	int	i;

	i = 0;
	while (i < env->num_coders)
	{
		pthread_mutex_lock(&env->coders[i].state_mutex);
		if (env->coders[i].compiles_done < env->num_compiles_required)
		{
			pthread_mutex_unlock(&env->coders[i].state_mutex);
			return (0);
		}
		pthread_mutex_unlock(&env->coders[i].state_mutex);
		i++;
	}
	return (1);
}

int	do_compile(t_coder *coder, t_env *env)
{
	if (check_simulation_end(env))
		return (0);
	pthread_mutex_lock(&coder->state_mutex);
	coder->last_compile_start = get_current_time();
	pthread_mutex_unlock(&coder->state_mutex);
	print_status(coder, COMPILE_CLR, "is compiling");
	codex_usleep(coder->env->time_to_compile, coder->env);
	pthread_mutex_lock(&coder->state_mutex);
	coder->compiles_done++;
	pthread_mutex_unlock(&coder->state_mutex);
	if (are_all_compiles_done(env))
		return (0);
	return (1);
}

int	do_debug(t_coder *coder, t_env *env)
{
	if (check_simulation_end(env))
		return (0);
	print_status(coder, DEBUG_CLR, "is debugging");
	codex_usleep(coder->env->time_to_debug, coder->env);
	return (1);
}

int	do_refactor(t_coder *coder, t_env *env)
{
	if (check_simulation_end(env))
		return (0);
	print_status(coder, REFACTOR_CLR, "is refactoring");
	codex_usleep(coder->env->time_to_refactor, coder->env);
	return (1);
}
