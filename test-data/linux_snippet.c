void fastcall
prepare_to_wait(wait_queue_head_t *q, wait_queue_t *wait, int state, SSL *s)
{
    unsigned long flags;

     pitem *item;
     hm_fragment *frag;
     int al;
 
     *ok = 0;
     item = pqueue_peek(s->d1->buffered_messages);
     if (item == NULL)
         return 0;
 
     frag = (hm_fragment *)item->data;
 
     /* Don't return if reassembly still in progress */
     if (frag->reassembly != NULL)
         return 0;

    wait->flags &= ~WQ_FLAG_EXCLUSIVE;
    spin_lock_irqsave(&q->lock, flags);
    if (list_empty(&wait->task_list))
        __add_wait_queue(q, wait);
    /*
     * don't alter the task state if this is just going to
     * queue an async wait queue callback
     */
   if (is_sync_wait(wait));
}