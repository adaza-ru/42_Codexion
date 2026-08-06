/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   prints.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/01 20:51:50 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/08/06 02:49:24 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

static char	*get_init_error_message(int err_mark)
{
	char	*error_messages[9];

	error_messages[0] = "Unexpected error initializing arbitrator mutex\n";
	error_messages[1] = "Unexpected error initializing write mutex\n";
	error_messages[2] = "Unexpected error initializing end mutex\n";
	error_messages[3] = "Unexpected error initializing coders cond\n";
	error_messages[4] = "Unexpected error initializing coders state mutex\n";
	error_messages[5] = "Unexpected error creating the watcher p_thread\n";
	error_messages[6] = "Unexpected error creating the coders p_threads\n";
	error_messages[7] = "\n";
	error_messages[8] = "\n";
	return (error_messages[err_mark]);
}

static char	*get_arg_error_message(int err_mark)
{
	char	*error_messages[9];

	error_messages[0] = "Wrong arguments format. Usage: \"./codexion"
		"<number_of_coders> <time_to_burnout>\n<time_to_compile>"
		"<time_to_debug> <time_to_refactor>\n<number_of_compiles_required>"
		" <dongle_cooldown> <scheduler>\"\n";
	error_messages[1] = "Wrong number of coders. Ir must be a positive"
		" integer between 1 and 250.\n";
	error_messages[2] = "Wrong time to burnout. It must be "
		"a positive non-zero integer.\n";
	error_messages[3] = "Wrong time to compile. It must be a"
		"positive non-zero integer.\n";
	error_messages[4] = "Wrong time to debug. It must be a "
		"positive non-zero integer.\n";
	error_messages[5] = "Wrong time to refactor. It must be a "
		"positive non-zero integer.\n";
	error_messages[6] = "Wrong number of compiles. It must be a "
		"positive non-zero integer.\n";
	error_messages[7] = "Wrong dongle cooldown. It must be a "
		"positive non-zero integer.\n";
	error_messages[8] = "Wrong scheduler. It must be \"edf\" or \"fifo\""
		"(Case Sensitive).\n";
	return (error_messages[err_mark]);
}

void	print_error(int err_mark)
{
	char	*err_messg;

	err_messg = NULL;
	if (err_mark < 10)
		err_messg = get_arg_error_message(err_mark - 1);
	if (err_mark > 10)
		err_messg = get_init_error_message(err_mark - 11);
	fprintf(stderr, DEATH_CLR "ERROR: %s" RESET, err_messg);
}

void	print_status(t_coder *coder, char *color, char *status)
{
	size_t	timestamp;

	pthread_mutex_lock(&coder->env->write_mutex);
	pthread_mutex_lock(&coder->env->end_mutex);
	if (!coder->env->simulation_end)
	{
		timestamp = get_current_time() - coder->env->start_time;
		fprintf(stdout, "%s%zu %d %s%s\n",
			color, timestamp, coder->id + 1, status, RESET);
	}
	pthread_mutex_unlock(&coder->env->end_mutex);
	pthread_mutex_unlock(&coder->env->write_mutex);
}
