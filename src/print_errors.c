/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   print_errors.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: adaza-ru <adaza-ru@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/14 17:30:44 by adaza-ru          #+#    #+#             */
/*   Updated: 2026/07/14 17:46:58 by adaza-ru         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "h_codexion.h"

void	print_error(int n)
{
	if (n == 1)
		fprintf(stderr, DEATH_CLR "Wrong arguments format. Usage: \"./codexion" 
			" <number_of_coders> <time_to_burnout> <time_to_compile>\n " 
			"<time_to_debug> <time_to_refactor> <number_of_compiles_required>"
			"<dongle_cooldown> <scheduler>\"\n" RESET);
}