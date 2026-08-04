/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder_actions.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 02:13:41 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/08/04 02:14:39 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

void	do_compile(t_coder *coder)
{
	pthread_mutex_lock(&coder->state_mutex);
	coder->last_compile_start = get_current_time();
	pthread_mutex_unlock(&coder->state_mutex);
	print_status(coder, COMPILE_CLR, "is compiling");
	codex_usleep(coder->env->time_to_compile, coder->env);
	pthread_mutex_lock(&coder->state_mutex);
	coder->compiles_done++;
	pthread_mutex_unlock(&coder->state_mutex);
}

void	do_debug(t_coder *coder)
{
	print_status(coder, DEBUG_CLR, "is debugging");
	codex_usleep(coder->env->time_to_debug, coder->env);
}

void	do_refactor(t_coder *coder)
{
	print_status(coder, REFACTOR_CLR, "is refactoring");
	codex_usleep(coder->env->time_to_refactor, coder->env);
}
